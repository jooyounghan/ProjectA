
[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	uint count;
	aliveIndexSet.GetDimensions(count);
}