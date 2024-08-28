var gbps_pred = {
	"$schema": "https://vega.github.io/schema/vega-lite/v5.json",
	"data": {"name": "myData"},
	"transform": [
		{"calculate": "datum.Inst / datum.Cyc", "as": "IPC"},
		{"calculate": "datum.Bytes / datum.Pkts", "as": "BPP"},
		{"calculate": "datum.Inst / datum.Pkts", "as": "IPP"},
		{"calculate": "datum.BPP * (1 / datum.IPP) * datum.IPC * 4800000000", "as": "bps"},
		{"calculate": "datum.bps / 1000000000", "as": "Gbps"}
	],
	"layer":[
		{
			"mark": "tick",
			"encoding": {
				"x": {"field": "Gbps", "type": "quantitative"},
				"y": {"field": "CPU"}
			}
		},
		{
			"mark": "tick",
			"encoding": {
				"x": {"aggregate":"average","field": "Gbps", "type": "quantitative"},
				"y": {"field": "CPU"},
				"color": {"value":"red"}
			}
		}
	]
}

