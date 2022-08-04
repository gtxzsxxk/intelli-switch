package main

import "github.com/gin-gonic/gin"

func main() {
	r := gin.Default()
	r.Delims("{[{", "}]}")
	r.LoadHTMLGlob("index.html")
	r.Static("/assets", "./assets")
	r.GET("/", index)
	r.Run() // 监听并在 0.0.0.0:8080 上启动服务
}
