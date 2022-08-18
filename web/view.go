package main

import (
	"encoding/json"

	"github.com/gin-contrib/sessions"
	"github.com/gin-gonic/gin"
)

func index(c *gin.Context) {
	c.HTML(200, "index.html", gin.H{})
}

func login(c *gin.Context) {
	session := sessions.Default(c)
	session.Options(sessions.Options{MaxAge: 14515200})
	username := c.Param("username")
	password := c.Param("password")
	var user_s User
	err := Db.First(&user_s, "username=?", username).Error
	if err != nil {
		session.Delete("username")
		c.String(200, "{\"msg\":\"No such an user.\"}")
		return
	}
	if password == user_s.Password {
		session.Set("username", username)
		session.Save()
		c.String(200, "{\"msg\":\"success\"}")
	} else {
		session.Delete("username")
		c.String(200, "{\"msg\":\"Invalid login attempt\"}")
	}
}

func getDevice(c *gin.Context) {
	if !hasLogin(c) {
		c.String(200, "{\"msg\":\"No permission.\"}")
		return
	}
	var devices []Device
	Db.Find(&devices)
	bytedata, _ := json.Marshal(devices)
	c.String(200, string(bytedata))
}

func hasLogin(c *gin.Context) bool {
	return true
	session := sessions.Default(c)
	username := session.Get("username")
	if username == nil {
		return false
	}
	var user_s User
	err := Db.First(&user_s, "username=?", username).Error
	return err == nil
}
