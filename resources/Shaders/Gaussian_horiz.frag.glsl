#version 430

layout (binding = 0) uniform sampler2D Texture;

in VertexData{
	vec2 TextureCoordinate;
}Input;

out vec4 fragmentColor;

uniform float weight[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{
	vec2 tex_offset = 1.0 / textureSize(Texture, 0);
	vec3 result = texture(Texture, Input.TextureCoordinate).rgb * weight[0];

	for(int i = 1; i < 5; ++i) {
	    result += texture(Texture, Input.TextureCoordinate + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
	    result += texture(Texture, Input.TextureCoordinate - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
	}
	fragmentColor = vec4(result, 1.0);
}