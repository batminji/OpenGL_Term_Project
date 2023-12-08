#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform vec3 lightPos;
uniform vec4 lightColor;
uniform vec4 objectColor;

uniform sampler2D outTexture;

void main ()
{
	if(texture(outTexture, TexCoord).a < 0.1)
        discard;

	float ambientLight = 0.3f;
	vec4 ambient = ambientLight * lightColor * objectColor; 

	vec3 normalVector = normalize(Normal);
	vec3 lightDir = normalize(lightPos - FragPos);

	float diffuseLight = max (dot (normalVector, lightDir), 0.0);
	
	vec4 diffuse = diffuseLight * lightColor * objectColor; 
	
	vec4 result = ambient + diffuse;

	FragColor = result;
	FragColor = FragColor * texture(outTexture, TexCoord);
	
}