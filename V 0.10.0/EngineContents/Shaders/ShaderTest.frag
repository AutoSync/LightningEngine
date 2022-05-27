#version 330 core
out vec4 FragColor;
  
in vec2 TexCoord;   //Cordenadas de textura

uniform sampler2D checkertx;
uniform sampler2D lightningtx;
uniform float u_time;
uniform vec2 u_resolution;

void main()
{   
    //vec2 uv = gl_FragCoord.xy / TexCoord.xy / u_resolution.xy;
    float t = u_time / 2.0;
    vec3 color = vec3(1.0);

    //color = vec3(uv.x, uv.y, 0.0);


    FragColor = mix(texture(checkertx, TexCoord), texture(lightningtx, TexCoord), 1.0);
    //FragColor = texture(lightningtx, TexCoord);
    //FragColor = vec4(vec3(uv.x, uv.y, abs(sin(t))),1.0);

    //FragColor = vec4(color,1.0);
} 