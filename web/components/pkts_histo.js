var pkts_histo = {
	"$schema": "https://vega.github.io/schema/vega-lite/v5.json",
	"data": {"name": "myData"},
	"mark": "bar",
	"encoding": {
		"x": {"field": "Pkts"},
		"y": {"aggregate": "count"},
		"column": {"field":"CPU"},
		"row": {"field":"Symbol"}
	}
}
