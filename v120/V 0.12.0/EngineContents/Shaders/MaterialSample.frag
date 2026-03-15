#version 330 core
out vec4 FragColor;

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
uniform vec3 ViewLocation;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragLocation;

uniform LightDirectional ldir; // Defines that we want to use a directional light
uniform Material material;
uniform bool renderNow;
uniform int renderMode;

float near = 0.1; 
float far  = 10.0; 
  
float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}
vec3 getSolidRender(LightDirectional ldir, vec3 norm);
vec4 SetLightDirectional(LightDirectional ldir, vec3 norm, vec3 ViewDirection);

void main()
{    
	vec3 norm = normalize(Normal);
	vec3 ViewDirection = normalize(ViewLocation - FragLocation);
	// Light Directional
	vec4 result = SetLightDirectional(ldir, norm, ViewDirection);

	//Light Point
	//for(int i = 0; i < NR_LIGHTS; i++)
			//Result += SetLightPoint(lpoint[i], norm, ViewDirection);
	//Light Spot
	//Result += SetSpotLight(lspot, norm, ViewDirection);

	switch(renderMode)
	{
	case 0:
		float depth = LinearizeDepth(gl_FragCoord.z) / far;
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
		vec3 SolidShader = getSolidRender(ldir, norm);
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
vec3 getSolidRender(LightDirectional ldir, vec3 norm)
{
	vec3 solidColor = vec3(0.8);
	vec3 ambient = solidColor * 0.2;
	vec3 LightDirection = normalize(-ldir.direction);
	float diff = max(dot(norm, LightDirection), 0.0);

	vec3 Diffuse = diff * vec3(0.95);
	vec3 solidShader = (ambient + Diffuse) * solidColor;
	return solidShader;
}
vec4 SetLightDirectional(LightDirectional ldir, vec3 norm, vec3 ViewDirection)
{		
		vec4 Result;

		vec3 LightDirection = normalize(-ldir.direction);
		float diff = max(dot(norm, LightDirection), 0.0);
		
		vec3 ReflectionDirection = reflect(-LightDirection, norm);
		float spec = pow(max(dot(ViewDirection, ReflectionDirection), 0.0), material.shininess);

		//vec3 ambient = ldir.ambient * vec3(texture(texture_diffuse_1, TexCoords));
		vec3 ambient = ldir.ambient * texture(material.diffuse, TexCoords).rgb;
		vec3 diffuse = ldir.diffuse * diff * texture(material.diffuse, TexCoords).rgb;
		
		vec3 specular = ldir.specular * spec * texture(material.specular, TexCoords).rgb;	
		Result = vec4(ambient + diffuse + specular, texture(material.specular, TexCoords).a);
	return Result;
}