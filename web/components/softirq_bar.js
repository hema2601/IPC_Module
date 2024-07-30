
var sirq_bar =
	{
		"$schema": "https://vega.github.io/schema/vega-lite/v5.json",
		"description": "NET_RX softirqs received per core",
		"data": {
		"name": "myData"
		},
		"mark": "bar",
		"transform": [
			{"calculate": "datum.After - datum.Before", "as": "softirqs"}
		],
		"encoding": {
			"x": {"field": "Core", "type": "nominal", "axis": {"labelAngle": 0}},
			"y": {"field": "softirqs", "type": "quantitative"},
			"xOffset":{"field": "Type"},
			"color":{"field":"Type"}
		}
	}
