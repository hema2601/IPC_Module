
var IPC_Histo = {
	"title":"Entry Counts",
	"$schema": "https://vega.github.io/schema/vega-lite/v5.json",
	"data": {
		"name": "myData"
	},
	"mark": "bar",
	"transform": [
		{"calculate": "datum.Inst / datum.Cyc", "as": "IPC"},
		{"filter": "datum.IPC < 5"},
		{"filter": "datum.Cyc < 16000000000000000000"},
	],
	"encoding": {
		"x": {
			"field": "CPU"
		},
		"y": {"aggregate": "count"},
		"column":{"field":"Symbol"},
		"color":{"field":"CPU"}
	}

}


