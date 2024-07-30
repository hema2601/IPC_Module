var IPC_Box =
	{
		"$schema": "https://vega.github.io/schema/vega-lite/v5.json",
		"description": "A vertical box plot showing median, min, and max body mass of penguins.",
		"data": {
		"name": "myData"
		},
		"mark": {
			"type": "boxplot",
			"extent": "min-max"
		},
		"transform": [
			{"calculate": "datum.Inst / datum.Cyc", "as": "IPC"},
			{"filter": "datum.IPC < 5"},
			{"filter": "datum.Cyc < 16000000000000000000"},
		],
		"encoding": {
			"x": {"field": "CPU", "type": "nominal"},
			"color": {"field": "CPU", "type": "nominal", "legend": null},
			"y": {
				"field": "IPC",
				"type": "quantitative",
			},
			"column":{"field":"Symbol"}
		}
	}


