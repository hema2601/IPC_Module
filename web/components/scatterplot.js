class Scatterplot{
	plot = {

	"title": "IPC of different Kernel Functions",
	"width": 400,
	"height": 400,
	"data": {
		"url":"data/with_napi/data.json"
	},
	"transform": [
		{"calculate": "datum.Inst / datum.Cyc", "as": "IPC"},
		{"filter": "datum.IPC < 5"},
		{"filter": "datum.Cyc < 16000000000000000000"},
		{"extent": "Cyc", "param": "MaxCyc"},
		{"extent": "Inst", "param": "MaxInst"}
	],
	"layer":[
		{
			"mark": "point",
			"transform": [
				{"calculate": "datum.Inst / datum.Cyc", "as": "IPC"},
				{"filter": "datum.IPC < 5"},
				{"filter": "datum.Cyc < 16000000000000000000"}//,
			],
			"encoding": {
				"x": {"field": "Cyc", "type": "quantitative"},
				"y": {"field": "Inst", "type": "quantitative"},
				"color": {"field": "CPU", "type": "nominal"},
				"shape": {"field": "CPU", "type": "nominal"},
				"row": {"field": "Symbol"}
			}
		},

		{
			"mark": {"type": "rule", "strokeDash": [3, 1]},
			"encoding": {

				"x": {"value": {"expr": "scale('x', 0)"}},
				"y": {"value": {"expr": "scale('y', 0)"}},
				"x2": {"value": {"expr": "scale('x', (MaxCyc[1] > MaxInst[1]) ? MaxInst[1] : MaxCyc[1])"}},
				"y2": {"value": {"expr": "scale('y', (MaxCyc[1] > MaxInst[1]) ? MaxInst[1] : MaxCyc[1])"}}
			}
		}
	]

}

	constructor(data, target){
		this.plot["data"]["url"] = data
		var filter = {}
		filter["filter"] = "datum.Symbol == '".concat(target).concat("'")
		this.plot["layer"][0]["transform"].push(filter)

	}

	getPlot(){
		return this.plot
	}

}
