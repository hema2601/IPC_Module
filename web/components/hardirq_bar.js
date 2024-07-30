
var hirq_bar =
	{
		"$schema": "https://vega.github.io/schema/vega-lite/v5.json",
		"description": "NET_RX softirqs received per core",
		"data": {
		"name": "myData"
		},
		"mark": "bar",
		"transform": [
			{"calculate": "datum.After - datum.Before", "as": "irqs"}
		],
		"encoding": {
			"x": {"field": "Queue", "type": "nominal", "axis": {"labelAngle": 0}},
			"y": {"field": "irqs", "type": "quantitative"},
		}
	}



