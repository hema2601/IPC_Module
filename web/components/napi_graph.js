
var napi_graph =
	{
		"$schema": "https://vega.github.io/schema/vega-lite/v5.json",
		"description": "Stock prices of four large companies as a small multiples of area charts.",
		"transform": [{"calculate": "datum.bps / 1000000000", "as": "Gbps"}],
		"data": {"name": "myData"},
		"facet": {"row": {"field": "NAPI_ID"}},
		"spec":{
			"layer":[
				{
					"mark": "line",
					"encoding": {
						"x": {
							"field": "t",
							"type": "temporal",
							"title": "Time",
							"axis": {"grid": false}
						},
						"y": {
							"field": "Gbps",
							"type": "quantitative",
							"axis": {"grid": false},
							"scale": {"zero": false}
						},
						"color": {"field": "Socket", "type": "nominal"}
					}
				},
				{
					"mark": "rule",
					"encoding": {
						"y": {
							"aggregate": "mean",
							"field": "Gbps",
							"type": "quantitative"
						},
						"color": {"value": "red"}
					}
				}
			]}
	}
