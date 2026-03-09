#version 330 core
#define MAX_LIGHT_POINTS 8

out vec4 FragColor;
in vec2 TexCoords;
in vec3 Normal;
in vec3 FragLocation;

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

struct LightPoint {    
    vec3 location;
    
    float constant;
    float linear;
    float quadratic;  

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  

uniform vec3 ViewLocation;

uniform LightDirectional LD; // Defines that we want to use a directional light
uniform LightPoint LP[MAX_LIGHT_POINTS];

//uniform LightSpot LS;
uniform Material material;
uniform bool renderNow;
uniform int renderMode;
uniform int numLights;
uniform bool lightpoints;

//support to Fog
uniform bool fogVisible;
uniform vec3 colorFog;
uniform float fogDensity;
uniform float fogNear; 
uniform float fogFar; 
  
float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * fogNear * fogFar) / (fogFar + fogNear - z * (fogFar - fogNear));	
}
vec3 getSolidRender(LightDirectional LD, vec3 norm);
vec3 getLightDirectional(LightDirectional light, vec3 normal, vec3 vl);
vec3 getLightPoint(LightPoint LP, vec3 normal, vec3 FragLocation,  vec3 vl);


void main()
{    
	vec3 norm = normalize(Normal);
	vec3 ViewDirection = normalize(ViewLocation - FragLocation);
	// Light Directional
	vec4 result = vec4(getLightDirectional(LD, norm, ViewDirection), 1.0);
	
	//Light Point
	for(int i = 0; i < numLights; i++)
			result += vec4(getLightPoint(LP[i], norm, FragLocation, ViewDirection), 1.0);
	//Light Spot
	//Result += SetSpotLight(lspot, norm, ViewDirection);
	switch(renderMode)
	{
	case 0:
		float depth = LinearizeDepth(gl_FragCoord.z) / fogFar;
		float intensity = 1.0;
		vec3 colorfog = vec3(0.79,0.82, 0.95);
		float fogDensity =  depth * intensity;
		vec3 fog = colorfog * fogDensity;
		vec4 environment = vec4(mix(result.rgb, fog, depth), texture(material.diffuse, TexCoords).a);
		FragColor = environment;
		break;
	case 1:
		FragColor = texture(material.diffuse, TexCoords);
		break;
	case 2:
		vec3 SolidShader = getSolidRender(LD, norm);
		FragColor = vec4(SolidShader, 1.0);
		break;
	case 3: 
		FragColor = result;
		break;
	default:
		FragColor = result;
		break;
	}
}
vec3 getSolidRender(LightDirectional light, vec3 norm)
{
	vec3 solidColor = vec3(0.8);
//	//vec3 ambient = solidColor * 0.2;
	vec3 ambient = texture(material.diffuse, TexCoords).rgb * 0.2;
	vec3 LightDirection = normalize(-LD.direction);
	float diff = max(dot(norm, LightDirection), 0.0);

	//vec3 Diffuse = diff * vec3(0.95);
	vec3 Diffuse = diff * texture(material.diffuse, TexCoords).rgb;
	vec3 solidShader = (ambient + Diffuse);
	return solidShader;
}
vec3 getLightDirectional(LightDirectional light, vec3 normal, vec3 vl)
{
	vec3 ambient, diffuse, specular, norm, lightdir, viewDir, reflecdir;
	float diff, spec;
	ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;
	lightdir = normalize(-light.direction);
	norm = normalize(normal);
	diff = max(dot(norm, lightdir),0.0);
	diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb;
	viewDir = normalize(vl - FragLocation);
	reflecdir = reflect(-lightdir, norm);
	spec = pow(max(dot(viewDir, reflecdir), 0.0), material.shininess);
	specular = light.specular * spec * texture(material.specular, TexCoords).rgb;

	return (ambient + diffuse + specular);
}

vec3 getLightPoint(LightPoint LP, vec3 normal, vec3 FragLocation,  vec3 vl)
{
	vec3 lightDir = normalize(LP.location - FragLocation);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(vl, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance    = length(LP.location - FragLocation);
    float attenuation = 1.0 / (LP.constant + LP.linear * distance + 
  			     LP.quadratic * (distance * distance));    
    // combine results
    vec3 ambient  = LP.ambient  * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse  = LP.diffuse  * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = LP.specular * spec * vec3(texture(material.specular, TexCoords));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}