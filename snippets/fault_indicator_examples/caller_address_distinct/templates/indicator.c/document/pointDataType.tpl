struct point_data
{
	void* caller_address;<$if concat(indicator.parameters.type)$>
	<$pointDataField : join(\n    )$><$endif$>
};