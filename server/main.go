package main

import (
	"context"
	"encoding/json"
	"html/template"
	"log"
	"net/http"
	"os"
	"sort"
	"strconv"
	"time"

	"github.com/jackc/pgx/v5/pgxpool"
)

type App struct {
	db *pgxpool.Pool
}

type JunctionCard struct {
	JunctionID int
	Name       string
	Lanes      []LaneStatus
}

type LaneStatus struct {
	LaneID        int
	Direction     string
	VehicleCount  int
	LightState    string
	GreenDuration int
	RedDuration   int
	UpdatedAt     string
}

type CorridorRow struct {
	CorridorName   string
	OrderIndex     int
	JunctionName   string
	DistanceToNext float64
}

type HistoryRow struct {
	LaneID       int
	Direction    string
	JunctionName string
	VehicleCount int
	RecordedAt   string
}

type DashboardData struct {
	Title     string
	Junctions []JunctionCard
	UpdatedAt string
}

type CorridorPageData struct {
	Title     string
	Rows      []CorridorRow
	UpdatedAt string
}

type HistoryPageData struct {
	Title     string
	Rows      []HistoryRow
	UpdatedAt string
}

type StatusResponse struct {
	UpdatedAt string         `json:"updated_at"`
	Junctions []JunctionCard `json:"junctions"`
}

const indexHTML = `
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>{{.Title}}</title>
  <style>
    :root {
      --bg: #f5f7fb;
      --card: #ffffff;
      --text: #1f2937;
      --muted: #6b7280;
      --border: #e5e7eb;
      --green: #16a34a;
      --red: #dc2626;
      --yellow: #ca8a04;
      --blue: #2563eb;
    }
    * { box-sizing: border-box; }
    body {
      margin: 0;
      font-family: Arial, sans-serif;
      background: var(--bg);
      color: var(--text);
    }
    header {
      background: var(--card);
      border-bottom: 1px solid var(--border);
      padding: 16px 24px;
      position: sticky;
      top: 0;
    }
    nav {
      display: flex;
      gap: 16px;
      align-items: center;
      justify-content: space-between;
      flex-wrap: wrap;
    }
    .nav-links {
      display: flex;
      gap: 12px;
      flex-wrap: wrap;
    }
    .nav-links a {
      text-decoration: none;
      color: var(--blue);
      font-weight: 600;
    }
    main {
      max-width: 1200px;
      margin: 0 auto;
      padding: 24px;
    }
    .meta {
      color: var(--muted);
      font-size: 14px;
      margin-bottom: 20px;
    }
    .grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(320px, 1fr));
      gap: 20px;
    }
    .card {
      background: var(--card);
      border: 1px solid var(--border);
      border-radius: 16px;
      padding: 18px;
      box-shadow: 0 4px 18px rgba(0,0,0,0.04);
    }
    .card h2 {
      margin: 0 0 14px;
      font-size: 20px;
    }
    .lane {
      border-top: 1px solid var(--border);
      padding: 12px 0;
    }
    .lane:first-of-type { border-top: none; }
    .row {
      display: flex;
      justify-content: space-between;
      gap: 8px;
      flex-wrap: wrap;
      margin: 4px 0;
    }
    .muted { color: var(--muted); }
    .badge {
      display: inline-block;
      padding: 4px 10px;
      border-radius: 999px;
      color: white;
      font-size: 12px;
      font-weight: 700;
      text-transform: uppercase;
      letter-spacing: 0.04em;
    }
    .green { background: var(--green); }
    .red { background: var(--red); }
    .yellow { background: var(--yellow); }
    .empty {
      background: var(--card);
      padding: 24px;
      border-radius: 16px;
      border: 1px dashed var(--border);
      color: var(--muted);
    }
  </style>
</head>
<body>
  <header>
    <nav>
      <div><strong>Smart Traffic Dashboard</strong></div>
      <div class="nav-links">
        <a href="/">Overview</a>
        <a href="/corridors">Corridors</a>
        <a href="/history">History</a>
      </div>
    </nav>
  </header>
  <main>
    <h1>Live Overview</h1>
    <p class="meta">Last refreshed: <span id="updatedAt">{{.UpdatedAt}}</span></p>

    {{if .Junctions}}
    <div class="grid">
      {{range .Junctions}}
      <section class="card">
        <h2>{{.Name}}</h2>
        {{range .Lanes}}
        <div class="lane">
          <div class="row">
            <strong>{{.Direction}}</strong>
            <span class="badge {{lower .LightState}}">{{.LightState}}</span>
          </div>
          <div class="row"><span class="muted">Lane ID</span><span>{{.LaneID}}</span></div>
          <div class="row"><span class="muted">Vehicles</span><span>{{.VehicleCount}}</span></div>
          <div class="row"><span class="muted">Green Time</span><span>{{.GreenDuration}} sec</span></div>
          <div class="row"><span class="muted">Red Time</span><span>{{.RedDuration}} sec</span></div>
          <div class="row"><span class="muted">Updated</span><span>{{.UpdatedAt}}</span></div>
        </div>
        {{end}}
      </section>
      {{end}}
    </div>
    {{else}}
    <div class="empty">No junction data found.</div>
    {{end}}
  </main>

  <script>
    async function refreshStatus() {
      try {
        const res = await fetch('/api/status');
        if (!res.ok) return;
        const data = await res.json();
        document.getElementById('updatedAt').textContent = data.updated_at;
        location.reload();
      } catch (err) {
        console.error(err);
      }
    }
    setInterval(refreshStatus, 5000);
  </script>
</body>
</html>
`

const corridorsHTML = `
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>{{.Title}}</title>
  <style>
    :root {
      --bg: #f5f7fb;
      --card: #ffffff;
      --text: #1f2937;
      --muted: #6b7280;
      --border: #e5e7eb;
      --blue: #2563eb;
    }
    * { box-sizing: border-box; }
    body {
      margin: 0;
      font-family: Arial, sans-serif;
      background: var(--bg);
      color: var(--text);
    }
    header {
      background: var(--card);
      border-bottom: 1px solid var(--border);
      padding: 16px 24px;
      position: sticky;
      top: 0;
    }
    nav {
      display: flex;
      gap: 16px;
      align-items: center;
      justify-content: space-between;
      flex-wrap: wrap;
    }
    .nav-links {
      display: flex;
      gap: 12px;
      flex-wrap: wrap;
    }
    .nav-links a {
      text-decoration: none;
      color: var(--blue);
      font-weight: 600;
    }
    main {
      max-width: 1200px;
      margin: 0 auto;
      padding: 24px;
    }
    .meta {
      color: var(--muted);
      font-size: 14px;
      margin-bottom: 20px;
    }
    table {
      width: 100%;
      border-collapse: collapse;
      background: var(--card);
      border-radius: 16px;
      overflow: hidden;
      border: 1px solid var(--border);
    }
    th, td {
      text-align: left;
      padding: 12px 14px;
      border-bottom: 1px solid var(--border);
    }
    th {
      background: #eef2ff;
    }
    tr:last-child td { border-bottom: none; }
    .empty {
      background: var(--card);
      padding: 24px;
      border-radius: 16px;
      border: 1px dashed var(--border);
      color: var(--muted);
    }
  </style>
</head>
<body>
  <header>
    <nav>
      <div><strong>Smart Traffic Dashboard</strong></div>
      <div class="nav-links">
        <a href="/">Overview</a>
        <a href="/corridors">Corridors</a>
        <a href="/history">History</a>
      </div>
    </nav>
  </header>
  <main>
    <h1>Corridor View</h1>
    <p class="meta">Last refreshed: {{.UpdatedAt}}</p>

    {{if .Rows}}
    <table>
      <thead>
        <tr>
          <th>Corridor</th>
          <th>Order</th>
          <th>Junction</th>
          <th>Distance to Next</th>
        </tr>
      </thead>
      <tbody>
        {{range .Rows}}
        <tr>
          <td>{{.CorridorName}}</td>
          <td>{{.OrderIndex}}</td>
          <td>{{.JunctionName}}</td>
          <td>{{printf "%.2f" .DistanceToNext}}</td>
        </tr>
        {{end}}
      </tbody>
    </table>
    {{else}}
    <div class="empty">No corridor rows found yet.</div>
    {{end}}
  </main>
</body>
</html>
`

const historyHTML = `
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>{{.Title}}</title>
  <style>
    :root {
      --bg: #f5f7fb;
      --card: #ffffff;
      --text: #1f2937;
      --muted: #6b7280;
      --border: #e5e7eb;
      --blue: #2563eb;
    }
    * { box-sizing: border-box; }
    body {
      margin: 0;
      font-family: Arial, sans-serif;
      background: var(--bg);
      color: var(--text);
    }
    header {
      background: var(--card);
      border-bottom: 1px solid var(--border);
      padding: 16px 24px;
      position: sticky;
      top: 0;
    }
    nav {
      display: flex;
      gap: 16px;
      align-items: center;
      justify-content: space-between;
      flex-wrap: wrap;
    }
    .nav-links {
      display: flex;
      gap: 12px;
      flex-wrap: wrap;
    }
    .nav-links a {
      text-decoration: none;
      color: var(--blue);
      font-weight: 600;
    }
    main {
      max-width: 1200px;
      margin: 0 auto;
      padding: 24px;
    }
    .meta {
      color: var(--muted);
      font-size: 14px;
      margin-bottom: 20px;
    }
    table {
      width: 100%;
      border-collapse: collapse;
      background: var(--card);
      border-radius: 16px;
      overflow: hidden;
      border: 1px solid var(--border);
    }
    th, td {
      text-align: left;
      padding: 12px 14px;
      border-bottom: 1px solid var(--border);
    }
    th {
      background: #eef2ff;
    }
    tr:last-child td { border-bottom: none; }
    .empty {
      background: var(--card);
      padding: 24px;
      border-radius: 16px;
      border: 1px dashed var(--border);
      color: var(--muted);
    }
  </style>
</head>
<body>
  <header>
    <nav>
      <div><strong>Smart Traffic Dashboard</strong></div>
      <div class="nav-links">
        <a href="/">Overview</a>
        <a href="/corridors">Corridors</a>
        <a href="/history">History</a>
      </div>
    </nav>
  </header>
  <main>
    <h1>Recent Sensor History</h1>
    <p class="meta">Last refreshed: {{.UpdatedAt}}</p>

    {{if .Rows}}
    <table>
      <thead>
        <tr>
          <th>Junction</th>
          <th>Lane</th>
          <th>Direction</th>
          <th>Vehicle Count</th>
          <th>Recorded At</th>
        </tr>
      </thead>
      <tbody>
        {{range .Rows}}
        <tr>
          <td>{{.JunctionName}}</td>
          <td>{{.LaneID}}</td>
          <td>{{.Direction}}</td>
          <td>{{.VehicleCount}}</td>
          <td>{{.RecordedAt}}</td>
        </tr>
        {{end}}
      </tbody>
    </table>
    {{else}}
    <div class="empty">No history records found yet.</div>
    {{end}}
  </main>
</body>
</html>
`

func main() {
	dsn := getenv("DATABASE_URL", "postgres://localhost/traffic_db?sslmode=disable")
	ctx := context.Background()

	pool, err := pgxpool.New(ctx, dsn)
	if err != nil {
		log.Fatalf("failed to create db pool: %v", err)
	}
	defer pool.Close()

	if err := pool.Ping(ctx); err != nil {
		log.Fatalf("failed to connect to db: %v", err)
	}

	app := &App{db: pool}

	http.HandleFunc("/", app.handleIndex)
	http.HandleFunc("/corridors", app.handleCorridors)
	http.HandleFunc("/history", app.handleHistory)
	http.HandleFunc("/api/status", app.handleStatusAPI)

	port := getenv("PORT", "8080")
	log.Printf("dashboard running at http://localhost:%s", port)
	log.Fatal(http.ListenAndServe(":"+port, nil))
}

func (a *App) handleIndex(w http.ResponseWriter, r *http.Request) {
	junctions, err := a.fetchJunctionCards(r.Context())
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	data := DashboardData{
		Title:     "Traffic Dashboard",
		Junctions: junctions,
		UpdatedAt: time.Now().Format("02 Jan 2006 15:04:05"),
	}

	tmpl := template.Must(template.New("index").Funcs(template.FuncMap{
		"lower": func(s string) string {
			switch s {
			case "GREEN", "Green", "green":
				return "green"
			case "YELLOW", "Yellow", "yellow":
				return "yellow"
			default:
				return "red"
			}
		},
	}).Parse(indexHTML))

	if err := tmpl.Execute(w, data); err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
	}
}

func (a *App) handleCorridors(w http.ResponseWriter, r *http.Request) {
	rows, err := a.fetchCorridors(r.Context())
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	data := CorridorPageData{
		Title:     "Corridors",
		Rows:      rows,
		UpdatedAt: time.Now().Format("02 Jan 2006 15:04:05"),
	}

	tmpl := template.Must(template.New("corridors").Parse(corridorsHTML))
	if err := tmpl.Execute(w, data); err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
	}
}

func (a *App) handleHistory(w http.ResponseWriter, r *http.Request) {
	rows, err := a.fetchHistory(r.Context())
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	data := HistoryPageData{
		Title:     "History",
		Rows:      rows,
		UpdatedAt: time.Now().Format("02 Jan 2006 15:04:05"),
	}

	tmpl := template.Must(template.New("history").Parse(historyHTML))
	if err := tmpl.Execute(w, data); err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
	}
}

func (a *App) handleStatusAPI(w http.ResponseWriter, r *http.Request) {
	junctions, err := a.fetchJunctionCards(r.Context())
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	resp := StatusResponse{
		UpdatedAt: time.Now().Format("02 Jan 2006 15:04:05"),
		Junctions: junctions,
	}

	w.Header().Set("Content-Type", "application/json")
	if err := json.NewEncoder(w).Encode(resp); err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
	}
}

func (a *App) fetchJunctionCards(ctx context.Context) ([]JunctionCard, error) {
	query := `
	SELECT
		j.junction_id,
		j.name,
		l.lane_id,
		l.direction,
		COALESCE(sr.vehicle_count, 0) AS vehicle_count,
		COALESCE(tl.state, 'RED') AS light_state,
		COALESCE(tl.green_time, 30) AS green_duration,
		COALESCE(tl.red_time, 30) AS red_duration,
		COALESCE(to_char(sr.reading_time, 'YYYY-MM-DD HH24:MI:SS'), '-') AS updated_at
	FROM junctions j
	JOIN lanes l
		ON l.junction_id = j.junction_id
	LEFT JOIN sensors s
		ON s.lane_id = l.lane_id
	LEFT JOIN LATERAL (
		SELECT sr1.vehicle_count, sr1.reading_time
		FROM sensor_readings sr1
		WHERE sr1.sensor_id = s.sensor_id
		ORDER BY sr1.reading_time DESC
		LIMIT 1
	) sr ON true
	LEFT JOIN traffic_lights tl
		ON tl.lane_id = l.lane_id
	ORDER BY j.junction_id, l.lane_id;
	`

	rows, err := a.db.Query(ctx, query)
	if err != nil {
		return nil, err
	}
	defer rows.Close()

	cardsMap := map[int]*JunctionCard{}
	order := []int{}

	for rows.Next() {
		var jc JunctionCard
		var lane LaneStatus

		if err := rows.Scan(
			&jc.JunctionID,
			&jc.Name,
			&lane.LaneID,
			&lane.Direction,
			&lane.VehicleCount,
			&lane.LightState,
			&lane.GreenDuration,
			&lane.RedDuration,
			&lane.UpdatedAt,
		); err != nil {
			return nil, err
		}

		card, ok := cardsMap[jc.JunctionID]
		if !ok {
			cardsMap[jc.JunctionID] = &JunctionCard{
				JunctionID: jc.JunctionID,
				Name:       jc.Name,
			}
			order = append(order, jc.JunctionID)
			card = cardsMap[jc.JunctionID]
		}

		card.Lanes = append(card.Lanes, lane)
	}

	if err := rows.Err(); err != nil {
		return nil, err
	}

	sort.Ints(order)
	result := make([]JunctionCard, 0, len(order))
	for _, id := range order {
		result = append(result, *cardsMap[id])
	}

	return result, nil
}

func (a *App) fetchCorridors(ctx context.Context) ([]CorridorRow, error) {
	query := `
	SELECT c.name, cn.order_index, j.name, cn.distance_to_next
	FROM corridor_nodes cn
	JOIN corridors c ON c.corridor_id = cn.corridor_id
	JOIN junctions j ON j.junction_id = cn.junction_id
	ORDER BY c.name, cn.order_index;
	`

	rows, err := a.db.Query(ctx, query)
	if err != nil {
		return nil, err
	}
	defer rows.Close()

	var result []CorridorRow
	for rows.Next() {
		var row CorridorRow
		if err := rows.Scan(&row.CorridorName, &row.OrderIndex, &row.JunctionName, &row.DistanceToNext); err != nil {
			return nil, err
		}
		result = append(result, row)
	}

	return result, rows.Err()
}

func (a *App) fetchHistory(ctx context.Context) ([]HistoryRow, error) {
	limit := 25
	queryParam := os.Getenv("HISTORY_LIMIT")
	if queryParam != "" {
		if v, err := strconv.Atoi(queryParam); err == nil && v > 0 {
			limit = v
		}
	}

	query := `
	SELECT
		l.lane_id,
		l.direction,
		j.name,
		sr.vehicle_count,
		to_char(sr.reading_time, 'YYYY-MM-DD HH24:MI:SS') AS recorded_at
	FROM sensor_readings sr
	JOIN sensors s
		ON s.sensor_id = sr.sensor_id
	JOIN lanes l
		ON l.lane_id = s.lane_id
	JOIN junctions j
		ON j.junction_id = l.junction_id
	ORDER BY sr.reading_time DESC
	LIMIT $1;
	`

	rows, err := a.db.Query(ctx, query, limit)
	if err != nil {
		return nil, err
	}
	defer rows.Close()

	var result []HistoryRow
	for rows.Next() {
		var row HistoryRow
		if err := rows.Scan(&row.LaneID, &row.Direction, &row.JunctionName, &row.VehicleCount, &row.RecordedAt); err != nil {
			return nil, err
		}
		result = append(result, row)
	}

	return result, rows.Err()
}

func getenv(key, fallback string) string {
	if v := os.Getenv(key); v != "" {
		return v
	}
	return fallback
}
