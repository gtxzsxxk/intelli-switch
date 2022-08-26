package main

import (
	"github.com/gin-gonic/gin"
)

func main() {
	db_initialize()
	// 创建基于cookie的存储引擎，secret11111 参数是用于加密的密钥
	r := gin.Default()
	r.Delims("{[{", "}]}")
	r.LoadHTMLGlob("index.html")
	r.Static("/assets", "./assets")
	r.GET("/", index)
	r.GET("/login/:username/:password", login)
	/* Operations on Device Level */
	r.GET("/devices", getDevice)
	r.POST("/devices/:name/:type/:icon", addDevice)
	r.DELETE("/devices/:name", deleteDevice)
	/* For management use */
	r.PUT("/device_property/:name", updateDeviceProperty)
	/* Operations on Device's Property Level */
	r.GET("/device_details/:name", getDeviceDetail)
	/* For IOT clients to add a new series of data */
	r.POST("/device_details/:name", addDeviceDetail)
	/* For Web Page to update single latest property's value */
	r.PUT("/device_details/:name", updateDeviceDetail)

	r.Run() // 监听并在 0.0.0.0:8080 上启动服务
}
