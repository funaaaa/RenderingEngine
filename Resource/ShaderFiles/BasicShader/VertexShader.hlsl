struct VSInput
{
    float4 m_position : POSITION;
    float4 m_color : COLOR;
};
struct VSOutput
{
    float4 m_position : SV_POSITION;
    float4 m_color : COLOR;
};

VSOutput main(VSInput arg_in)
{
    
    VSOutput result = (VSOutput) 0;
    result.m_position = arg_in.m_position;
    result.m_color = arg_in.m_color;
    return result;
    
}