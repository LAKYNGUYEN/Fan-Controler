
#ifndef MAIN_PAGE_H
#define MAIN_PAGE_H
const char *MAIN_PAGE = R"html(<!doctypehtml><html lang=en><meta charset=UTF-8><meta content=IE=edge http-equiv=X-UA-Compatible><meta content=width=device-width,initial-scale=1.0 name=viewport><title>Fan controller</title><style>body{background-image:linear-gradient(135deg,#50cdd1 10%,#523221);background-repeat:no-repeat;height:100vh;font-family:Georgia,Times New Roman,Times,serif}.container{text-align:center;border:3px solid #ccc;border-radius:10px;justify-content:center;width:50%;margin:200px auto auto;padding:20px 1px;box-shadow:inset 100px 200px 100px #cdd4d4}.container>h1{margin-bottom:50px}.fan>button{cursor:pointer;border:none;border-radius:10px;margin:10px;padding:10px 20px;font-size:14px;box-shadow:2px 2px 5px #fc894d}.fan>button.active{color:#fff;cursor:pointer;background-image:linear-gradient(135deg,#40d2f3 40%,#be6434);border:none;border-radius:10px;font-weight:700}.sensor-data{text-align:center;border:3px solid #ccc;border-radius:10px;justify-content:center;width:50%;margin:50px auto auto;padding:20px 1px;font-size:15px;box-shadow:inset 100px 150px 100px #cdd4d4}</style><body><div class=container><h1>Fan controller</h1><div class=fan><button id=fan>Fan</button><button id=light>Light</button><button id=ON>ON</button></div></div><div class=sensor-data><p>Nhiệt độ: <span id=temperature>Đang tải...</span>°C<p>Độ ẩm: <span id=humidity>Đang tải...</span>%</div><script>document.querySelectorAll("button").forEach(button => {
            button.addEventListener("click", () => {
                const id = button.id;
                const state = button.classList.contains("active") ? "OFF" : "ON";
                fetch(`/${id}?state=${state}`);
            });
        
        });</script><script>console.log("Connecting to", `ws://${location.host}:81`)
        var ws = new WebSocket(`ws://${location.host}:81`)
        ws.onopen = function() {
            console.log("Connected")
            ws.send("GET")
        }
        ws.onmessage = function(event) {
            console.log(event.data)
            event.data.split("\n").forEach(line => {
                const [id, value] = line.split(":")
                const dev = document.getElementById(id)
                console.log('id', id, 'value', value)
                console.log(dev)
                if (dev) {
                    if (value == "ON") {
                        dev.classList.add("active")
                    } else {
                        dev.classList.remove("active")
                    }
                } else {
                    // Xử lý dữ liệu từ DHT
                    if (id === "temperature" || id === "humidity") {
                        const sensorElement = document.getElementById(id)
                        if (sensorElement) {
                            sensorElement.innerText = value
                        }
                    }
                }
            })
        }
        
        ws.onerror = function(event) {
            console.error("WebSocket error observed:", event)    
        }

        //get data from server</script><script>//get data from server
       function fetchData() {
            fetch("/data")
                .then(response => response.json())
                .then(data => {
                    document.getElementById("temperature").innerText = data.temperature
                    document.getElementById("humidity").innerText = data.humidity
                })
        }
        setInterval(fetchData, 1000)</script> )html";
#endif // MAIN_PAGE_H
