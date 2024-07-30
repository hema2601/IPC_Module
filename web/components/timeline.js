
var IPC_Time = {
	"title": "IPC Over Time",
	"width": 10000,
	"height": 80,
	"data": {
		"name": "myData"
	},
	"mark": "line",
	"transform": [
		{"calculate": "datum.Inst / datum.Cyc", "as": "IPC"},

		{"filter": "datum.IPC < 5"},
		{"filter": "datum.Cyc < 16000000000000000000"},
	],
	"encoding": {
		"x": {"field": "t", "type": "temporal"},
		"y": {"field": "IPC", "type": "quantitative"},
		"color": {"field": "CPU", "type": "nominal"},
		"row": {"field": "Symbol"}
	}
}
