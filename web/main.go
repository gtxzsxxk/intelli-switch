package main

import (
	"github.com/gin-contrib/sessions"
	"github.com/gin-contrib/sessions/cookie"
	"github.com/gin-gonic/gin"
)

func main() {
	db_initialize()
	// 创建基于cookie的存储引擎，secret11111 参数是用于加密的密钥
	r := gin.Default()
	store := cookie.NewStore([]byte("cdasgdabu"))
	r.Use(sessions.Sessions("swsession", store))
	r.Delims("{[{", "}]}")
	r.LoadHTMLGlob("index.html")
	r.Static("/assets", "./assets")
	r.GET("/", index)
	r.GET("/login/:username/:password", login)
	r.GET("/device", getDevice)
	r.Run() // 监听并在 0.0.0.0:8080 上启动服务
}
