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
struct LightPoint
{
	vec3 location; 

	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
struct SpotLight
{
	vec3 location;
	vec3 direction;
	float cutOff;
	float outerCutOff;

	float constant;
	float linear;
	float quadratic;

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
#define MAX_LIGHT_POINTS 8 
uniform int NR_LIGHTS;
uniform LightPoint lpoint[MAX_LIGHT_POINTS]; //Defines that we want to use points
uniform SpotLight lspot;
uniform Material material;
//Functions
vec3 SetLightDirectional(LightDirectional ldir, vec3 norm, vec3 ViewDirection);
vec3 SetLightPoint(LightPoint lpoint, vec3 norm, vec3 ViewDirection);
vec3 SetSpotLight(SpotLight lspot, vec3 norm, vec3 ViewDirection);

void main()
{	
	vec3 norm = normalize(Normal);
	vec3 ViewDirection = normalize(ViewLocation - FragLocation);
	// Light Directional
	vec3 Result = SetLightDirectional(ldir, norm, ViewDirection);
	//Light Point
	for(int i = 0; i < NR_LIGHTS; i++)
			Result += SetLightPoint(lpoint[i], norm, ViewDirection);
	//Light Spot
	//Result += SetSpotLight(lspot, norm, ViewDirection);

	FragColor = vec4(Result, 1.0);
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

		vec3 ambient = ldir.ambient * vec3(texture(material.diffuse, TexCoords));			//Ambient
		vec3 diffuse = ldir.diffuse * diff *vec3(texture(material.diffuse, TexCoords));		//Diffuse
		vec3 specular = ldir.specular * spec *vec3(texture(material.specular, TexCoords));	//Specular
		Result = ambient + diffuse + specular;
	return Result;
}
vec3 SetLightPoint(LightPoint lpoint, vec3 norm, vec3 ViewDirection)
{
	vec3 LightDirection = normalize(lpoint.location - FragLocation);
	//diffuse
	float diff = max(dot(norm, LightDirection), 0.0);
	//specular
	vec3 ReflectionDirection = reflect(-LightDirection, norm);
	float spec = pow(max(dot(ViewDirection, ReflectionDirection), 0.0),material.shininess);
	// attenuation
	float dist = length(lpoint.location - FragLocation);
	float attenuation = 1.0 / (lpoint.constant + lpoint.linear * dist + 
								lpoint.quadratic * (dist * dist));
	//Combine Results
	vec3 ambient = lpoint.ambient * vec3(texture(material.diffuse, TexCoords));
	vec3 diffuse = lpoint.diffuse * diff *  vec3(texture(material.diffuse, TexCoords));
	vec3 specular = lpoint.specular * spec *  vec3(texture(material.specular, TexCoords));
	
	ambient		*= attenuation;
	diffuse		*= attenuation;
	specular	*= attenuation;
	vec3 result = ambient + diffuse + specular;
	return result;
}

vec3 SetSpotLight(SpotLight lspot, vec3 norm, vec3 ViewDirection)
{
	vec3 result = vec3(1.0);
	vec3 LightDirection = normalize(lspot.location - FragLocation);
	// diffuse shading
	float diff = max(dot(norm, LightDirection), 0.0);
	// specular shading
	vec3 ReflectionDirection = reflect(-LightDirection, norm);
	float spec = pow(max(dot(ViewDirection, ReflectionDirection),0.0), material.shininess);
	// attenuation
	float dist = length(lspot.location - FragLocation);
	float attenuation = 1.0 / (lspot.constant + lspot.linear * dist + 
								lspot.quadratic * (dist * dist));
	// spotlight intensity
	float theta = dot(LightDirection, normalize(-lspot.direction));
	float epsilon = lspot.cutOff - lspot.outerCutOff;
	float intensity = clamp((theta - lspot.cutOff) / epsilon, 0.0, 1.0);
	
	vec3 ambient = lspot.ambient * vec3(texture(material.diffuse, TexCoords));
	vec3 diffuse = lspot.diffuse * vec3(texture(material.diffuse, TexCoords));
	vec3 specular = lspot.specular * vec3(texture(material.specular, TexCoords));

	ambient *= attenuation * intensity;
	diffuse *= attenuation  * intensity;
	specular *= attenuation * intensity;
	result = ambient + diffuse + specular;

	return result;
}