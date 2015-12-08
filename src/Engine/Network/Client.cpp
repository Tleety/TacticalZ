#include "Network/Client.h"

using namespace boost::asio::ip;


Client::Client() : m_Socket(m_IOService)
{
	// Set up network stream
	m_ReceiverEndpoint = udp::endpoint(boost::asio::ip::address::from_string("192.168.1.6"), 13);
}

Client::~Client()
{
    // will it work on linux? #if defined (_WIN64) | defined(_WIN32) otherwise.
    _CrtDumpMemoryLeaks();
}

void Client::Start(World* world, EventBroker* eventBroker)
{
	// Subscribe to events
	m_EventBroker = eventBroker;
	m_World = world;
	m_EKeyDown = decltype(m_EKeyDown)(std::bind(&Client::OnKeyDown, this, std::placeholders::_1));
	m_EventBroker->Subscribe(m_EKeyDown);
	std::cout << "Please enter you name: ";
	std::cin >> m_PlayerName;
	while (m_PlayerName.size() > 7) {
		std::cout << "Please enter you name(No longer than 7 characters): ";
		std::cin >> m_PlayerName;
	}
	m_Socket.connect(m_ReceiverEndpoint);
	std::cout << "I am client. BIP BOP\n";

    ReadFromServer();
}

void Client::Close()
{
    Disconnect();
    m_ThreadIsRunning = false;
    m_Socket.close();
}

void Client::ReadFromServer()
{
	int bytesRead = -1;
	char readBuf[1024] = { 0 };

	while (m_ThreadIsRunning) {
		if (m_Socket.available()) {
			bytesRead = Receive(readBuf, INPUTSIZE);
			ParseMessageType(readBuf, bytesRead);
		}
	}
}

void Client::ParseMessageType(char* data, size_t length)
{
	int messageType = -1;
	memcpy(&messageType, data, sizeof(int)); // Read what type off message was sent from server
	MoveMessageHead(data, length, sizeof(int)); // Move the message head to know where to read from

	switch (static_cast<MessageType>(messageType)) {
	case MessageType::Connect:
		ParseConnect(data, length);
		break;
	case MessageType::ClientPing:
		ParsePing();
		break;
	case MessageType::ServerPing:
		ParseServerPing();
		break;
	case MessageType::Message:
		break;
	case MessageType::Snapshot:
		ParseSnapshot(data, length);
		break;
	case MessageType::Disconnect:
		break;
	case MessageType::Event:
		ParseEventMessage(data, length);
		break;
	default:
		break;
	}
}

void Client::ParseConnect(char* data, size_t len)
{
	memcpy(&m_PlayerID, data, sizeof(int));
	std::cout << "I am player: " << m_PlayerID << std::endl;
}

void Client::ParsePing()
{
	m_DurationOfPingTime = 1000 * (std::clock() - m_StartPingTime) / static_cast<double>(CLOCKS_PER_SEC);
	std::cout << "response time with ctime(ms): " << m_DurationOfPingTime << std::endl;
}

void Client::ParseServerPing()
{
	char* testMessage = new char[128];
	int testOffset = CreateMessage(MessageType::ServerPing, "Ping recieved", testMessage);

	//std::cout << "Parsing ping." << std::endl;

	m_Socket.send_to(boost::asio::buffer(
        testMessage,
		testOffset),
		m_ReceiverEndpoint, 0);
    delete[] testMessage;
}

void Client::ParseEventMessage(char* data, size_t length)
{
	int Id = -1;
	std::string command = std::string(data);
	if (command.find("+Player") != std::string::npos) {
		MoveMessageHead(data, length, command.size() + 1);
		memcpy(&Id, data, sizeof(int));
		MoveMessageHead(data, length, sizeof(int));
		// Sett Player name
		m_PlayerDefinitions[Id].Name = command.erase(0, 7);
	}
	else {
		std::cout << "Event message: " << std::string(data) << std::endl;
	}

	MoveMessageHead(data, length, std::string(data).size() + 1);
}

void Client::ParseSnapshot(char* data, size_t length)
{
	std::string tempName;
	for (size_t i = 0; i < MAXCONNECTIONS; i++) {
		// We're checking for empty name for now. This might not be the best way,
		// but it is to avoid sending redundant data.

		// Read position data
		glm::vec3 playerPos;
		memcpy(&playerPos.x, data, sizeof(float));
		MoveMessageHead(data, length, sizeof(float));
		memcpy(&playerPos.y, data, sizeof(float));
		MoveMessageHead(data, length, sizeof(float));
		memcpy(&playerPos.z, data, sizeof(float));
		MoveMessageHead(data, length, sizeof(float));
		
		tempName = std::string(data);
		// +1 for null terminator
		MoveMessageHead(data, length, tempName.size() + 1);
		// Apply the position data read to the player entity
		// New player connected on the server side 
		if (m_PlayerDefinitions[i].Name == "" && tempName != "") { 
			CreateNewPlayer(i);
		}
		else if (m_PlayerDefinitions[i].Name != "" && tempName == "") {
			// Someone disconnected
			// TODO: Insert code here
		}
		else {
			// Not a connected player
			break;
		}
		m_World->GetComponent(m_PlayerDefinitions[i].EntityID, "Transform")["Position"] = playerPos;
		m_PlayerDefinitions[i].Name = tempName;
	}
}

int Client::Receive(char* data, size_t length)
{
	int bytesReceived = m_Socket.receive_from(boost
		::asio::buffer((void*)data, length),
		m_ReceiverEndpoint,
		0);
	return bytesReceived;
}

int Client::CreateMessage(MessageType type, std::string message, char* data)
{
	int lengthOfMessage = 0;
	int messageType = static_cast<int>(type);
	lengthOfMessage = message.size();

	int offset = 0;
	// Message type
	memcpy(data + offset, &messageType, sizeof(int));
	offset += sizeof(int);
	// Message, add one extra byte for null terminator
	memcpy(data + offset, message.data(), (lengthOfMessage + 1) * sizeof(char));
	offset += (lengthOfMessage + 1) * sizeof(char);

	return offset;
}

void Client::Connect()
{
    char* dataPackage = new char[INPUTSIZE]; // The package that will be sent to the server, when filled
    int length = CreateMessage(MessageType::Connect, m_PlayerName, dataPackage);
    m_StartPingTime = std::clock();
    m_Socket.send_to(boost::asio::buffer(
        dataPackage,
        length),
        m_ReceiverEndpoint, 0);
    delete[] dataPackage;
}

void Client::Disconnect()
{
    char* dataPackage = new char[INPUTSIZE]; // The package that will be sent to the server, when filled
    int len = CreateMessage(MessageType::Disconnect, "+Disconnect", dataPackage);
    m_Socket.send_to(boost::asio::buffer(
        dataPackage,
        len),
        m_ReceiverEndpoint, 0);
    delete[] dataPackage;
}

void Client::Ping()
{ 
    char* dataPackage = new char[INPUTSIZE];
    if (GetAsyncKeyState('P')) { // Maybe use previous key here
        int length = CreateMessage(MessageType::ClientPing, "Ping", dataPackage);
        m_StartPingTime = std::clock();
        m_Socket.send_to(boost::asio::buffer(
            dataPackage,
            length),
            m_ReceiverEndpoint, 0);
    }
    memset(dataPackage, 0, INPUTSIZE);
    delete[] dataPackage;
}

void Client::MoveMessageHead(char*& data, size_t& length, size_t stepSize)
{
	data += stepSize;
	length -= stepSize;
}

bool Client::OnKeyDown(const Events::KeyDown& event)
{
	char* dataPackage = new char[INPUTSIZE]; // The package that will be sent to the server, when filled
	if (event.KeyCode == GLFW_KEY_W) {
		int len = CreateMessage(MessageType::Event, "+Forward", dataPackage);
		m_Socket.send_to(boost::asio::buffer(
			dataPackage,
			len),
			m_ReceiverEndpoint, 0);
	}
	if (event.KeyCode == GLFW_KEY_A) {
		int len = CreateMessage(MessageType::Event, "-Right", dataPackage);
		m_Socket.send_to(boost::asio::buffer(
			dataPackage,
			len),
			m_ReceiverEndpoint, 0);
	}
	if (event.KeyCode == GLFW_KEY_S) {
		int len = CreateMessage(MessageType::Event, "-Forward", dataPackage);
		m_Socket.send_to(boost::asio::buffer(
			dataPackage,
			len),
			m_ReceiverEndpoint, 0);
	}
	if (event.KeyCode == GLFW_KEY_D) {
		int len = CreateMessage(MessageType::Event, "+Right", dataPackage);
		m_Socket.send_to(boost::asio::buffer(
			dataPackage,
			len),
			m_ReceiverEndpoint, 0);
	}
	if (event.KeyCode == GLFW_KEY_V) {
		Disconnect();
	}
	if (event.KeyCode == GLFW_KEY_C) {
		Connect();
	}
    if (event.KeyCode == GLFW_KEY_P) {
        Ping();
    }
	memset(dataPackage, 0, INPUTSIZE);
	delete[] dataPackage;
	return true;
}

void Client::CreateNewPlayer(int i)
{
	m_PlayerDefinitions[i].EntityID = m_World->CreateEntity();
	ComponentWrapper transform = m_World->AttachComponent(m_PlayerDefinitions[i].EntityID, "Transform");
	ComponentWrapper model = m_World->AttachComponent(m_PlayerDefinitions[i].EntityID, "Model");
	model["Resource"] = "Models/Core/UnitSphere.obj";
}