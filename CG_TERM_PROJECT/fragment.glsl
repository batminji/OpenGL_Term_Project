#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform vec3 lightPos;
uniform vec3 lightColor;

uniform sampler2D outTexture;

void main ()
{
	if(texture(outTexture, TexCoord).a < 0.1)
        discard;

	float ambientLight = 0.3f;
	vec3 ambient = ambientLight * lightColor;

	vec3 normalVector = normalize(Normal);
	vec3 lightDir = normalize(lightPos - FragPos);

	float diffuseLight = max (dot (normalVector, lightDir), 0.0);
	
	vec3 diffuse = diffuseLight * lightColor;
	
	vec3 result = ambient + diffuse;

	FragColor = vec4 (result, 1.0);
	FragColor = FragColor * texture(outTexture, TexCoord);
	
}