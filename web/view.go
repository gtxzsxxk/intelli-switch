package main

import (
	"encoding/base64"
	"encoding/json"
	"strconv"
	"strings"
	"time"

	"github.com/gin-contrib/sessions"
	"github.com/gin-gonic/gin"
	"github.com/xxtea/xxtea-go/xxtea"
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
	for i, v := range devices {
		device_id := v.ID
		var property Property
		err := Db.Last(&property, "device_id=?", device_id).Error
		if err != nil {
			devices[i].Online = 0
			continue
		}
		var property_value PropertyValue
		err = Db.Last(&property_value, "property_id=?", property.ID).Error
		if err != nil {
			devices[i].Online = 0
			continue
		}
		t_now := time.Now()
		elapsed := t_now.Sub(property_value.UpdatedAt).Seconds()
		if elapsed > 120 {
			devices[i].Online = 0
		} else {
			devices[i].Online = 1
			break
		}
	}
	bytedata, _ := json.Marshal(devices)
	c.String(200, string(bytedata))
}

func getDeviceDetail(c *gin.Context) {
	if !hasLogin(c) {
		c.String(200, "{\"msg\":\"No permission.\"}")
		return
	}
	var device Device
	device_name := c.Param("name")
	Db.First(&device, "name=?", device_name)
	var properties []Property
	Db.Where("device_id=?", device.ID).Find(&properties)
	type DeviceDetail struct {
		Property
		PropertyValue
	}
	var dd_list []DeviceDetail
	for _, v := range properties {
		tmp_dd := DeviceDetail{}
		tmp_dd.Type = v.Type
		tmp_dd.Name = v.Name
		var tmp_d_value PropertyValue
		Db.Last(&tmp_d_value, "property_id=?", v.ID)
		if v.Type == "map" {
			var tmp_d_value PropertyValue
			Db.Last(&tmp_d_value, "property_id=?", v.ID)
			tm_of_the_day := tmp_d_value.UpdatedAt
			tm_of_the_day_begin := time.Date(tm_of_the_day.Year(), tm_of_the_day.Month(), tm_of_the_day.Day(), 0, 0, 0, 0, tm_of_the_day.Location())
			tm_of_the_day_end := time.Date(tm_of_the_day.Year(), tm_of_the_day.Month(), tm_of_the_day.Day(), 23, 59, 59, 0, tm_of_the_day.Location())
			var results []PropertyValue
			err := Db.Where("property_id=? AND updated_at BETWEEN ? AND ?", v.ID, tm_of_the_day_begin, tm_of_the_day_end).Find(&results).Error
			if err != nil {
				tmp_dd.Value = "[[0,0]]"
			}
			var tmp_values []string
			for _, v := range results {
				tmp_values = append(tmp_values, v.Value)
			}
			tmp_byt, _ := json.Marshal(tmp_values)
			tmp_value_result := strings.ReplaceAll(string(tmp_byt), "\"", "")
			tmp_dd.Value = tmp_value_result
		} else if v.Type == "chart" {

		} else {
			tmp_dd.Value = tmp_d_value.Value
		}
		tmp_dd.Size = v.Size
		tmp_dd.Icon = v.Icon
		tmp_dd.Unit = v.Unit
		tmp_dd.PropertyID = tmp_d_value.PropertyID
		dd_list = append(dd_list, tmp_dd)
	}
	bytedata, _ := json.Marshal(dd_list)
	c.String(200, string(bytedata))
	/*properties := strings.Split(device.Property, ",")
	var tmp_device_detail DeviceDetails
	var deviceDetails []DeviceDetails
	for _, v := range properties {
		Db.Where("device_id=? AND name=?", device.ID, v).Last(&tmp_device_detail)
		deviceDetails = append(deviceDetails, tmp_device_detail)
	}
	bytedata, _ := json.Marshal(deviceDetails)
	c.String(200, string(bytedata))*/
}

func updateDeviceDetail(c *gin.Context) {
	property_id := c.Param("property_id")
	value := c.PostForm("value")
	usr_real := c.PostForm("username")
	value_inkey := c.PostForm("signature")
	var user User
	if err := Db.First(&user, "username=?", usr_real).Error; err != nil {
		c.String(401, "{\"msg\":\"No permission.\"}")
		return
	}
	value_tmp_bytes, _ := base64.StdEncoding.DecodeString(value_inkey)
	/* encrypt_data := xxtea.Encrypt([]byte("29.3"), []byte(user.Password[0:16]))*/
	/* fmt.Println(encrypt_data) */
	value_decrypt := string(xxtea.Decrypt(value_tmp_bytes, []byte(user.Password[0:16])))
	if value_decrypt != value {
		c.String(401, "{\"msg\":\"No permission.\"}")
		return
	}
	var property Property
	if err := Db.First(&property, "ID=?", property_id).Error; err != nil {
		c.String(400, "{\"msg\":\"No such a property.\"}")
		return
	}
	prop_id_int, _ := strconv.ParseInt(property_id, 10, 64)
	Db.Create(&PropertyValue{PropertyID: int(prop_id_int), Value: value})
	c.String(200, "{\"msg\":\"success\"}")
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
