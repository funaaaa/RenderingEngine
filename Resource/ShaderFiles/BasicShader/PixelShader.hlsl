struct VSOutput
{
    float4 m_position : SV_POSITION;
    float4 m_color : COLOR;
};

float4 main(VSOutput arg_in) : SV_TARGET
{
    return arg_in.m_color;
}