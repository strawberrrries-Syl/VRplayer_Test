#version 330 core
in vec3 ourColor;
in vec2 TexCoord;


out vec4 color;

uniform sampler2D ourTexture;  //�������Զ���ֵ Y 
uniform sampler2D ourTexture2;  //�������Զ���ֵ U
uniform sampler2D ourTexture3;  //�������Զ���ֵ V



void main()
{
    vec3 yuv;
    vec3 rgb;    
    yuv.x = texture2D(ourTexture, TexCoord).r;
    yuv.y = texture2D(ourTexture2, TexCoord).r - 0.5;
    yuv.z = texture2D(ourTexture3, TexCoord).r - 0.5;
    rgb = mat3( 1,       1,         1,
                0,       -0.39465,  2.03211,
                1.13983, -0.58060,  0) * yuv; 

     color = vec4(rgb, 1);
    //ԭ����ֻ��ourTexture
    //color = texture(ourTexture + ourTexture2 + ourTexture3, TexCoord);
}