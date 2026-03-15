#version 330 core
//Outputs
out vec4 FragColor;
//Structs
struct Material
{
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};
struct LightDirectional
{
	vec3 direction; 

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

//Inputs
in vec2 TexCoords;
in vec3 Normal;
in vec3 FragLocation;

uniform int TypeLight; // Switch between light source types
uniform vec3 ViewLocation;

uniform LightDirectional ldir; // Defines that we want to use a directional light
uniform Material material;
//
uniform sampler2D boat_diffuse;
uniform sampler2D boat_specular;
uniform sampler2D boat_normal;
uniform sampler2D boat_ambient;
//Functions
vec3 SetLightDirectional(LightDirectional ldir, vec3 norm, vec3 ViewDirection);

void main()
{	
	vec3 norm = normalize(Normal);
	vec3 ViewDirection = normalize(ViewLocation - FragLocation);
	// Light Directional
	vec3 Result = SetLightDirectional(ldir, norm, ViewDirection);
	//Light Point
	//for(int i = 0; i < NR_LIGHTS; i++)
			//Result += SetLightPoint(lpoint[i], norm, ViewDirection);
	//Light Spot
	//Result += SetSpotLight(lspot, norm, ViewDirection);

	//FragColor = vec4(Result,1.0);
	FragColor = texture(boat_diffuse, TexCoords);
}

//////////////////////////////////////////////////////////////////////////
//Functions
vec3 SetLightDirectional(LightDirectional ldir, vec3 norm, vec3 ViewDirection)
{		
		vec3 Result = vec3(1.0);

		vec3 LightDirection = normalize(-ldir.direction);
		float diff = max(dot(norm, LightDirection), 0.0);
		
		vec3 ReflectionDirection = reflect(-LightDirection, norm);
		float spec = pow(max(dot(ViewDirection, ReflectionDirection), 0.0), material.shininess);

		//vec3 ambient = ldir.ambient * vec3(texture(texture_diffuse_1, TexCoords));
		vec3 ambient = ldir.ambient * texture(material.diffuse, TexCoords).rgb;
		vec3 diffuse = ldir.diffuse * diff * texture(material.diffuse, TexCoords).rgb;		
		vec3 specular = ldir.specular * spec * texture(material.specular, TexCoords).rgb;	
		Result = ambient + diffuse + specular;
	return Result;
}
