var pkt_bar =
	{
		"$schema": "https://vega.github.io/schema/vega-lite/v5.json",
		"description": "Pkts received per NIC Queue",
		"data": {
		"name": "myData"
		},
		"mark": "bar",
		"transform": [
			{"calculate": "datum.After - datum.Before", "as":"pkts"}
		],
		"encoding": {
			"x": {"field": "Queue", "type": "nominal", "axis": {"labelAngle": 0}},
			"y": {"field": "pkts", "type": "quantitative"},
			"xOffset":{"field": "Type"},
			"color":{"field":"Type"}
		}
	}
