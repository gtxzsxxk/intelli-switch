package main

import (
	"gorm.io/driver/sqlite"
	"gorm.io/gorm"
)

type User struct {
	gorm.Model
	Username   string `gorm:"size:32;unique:true"`
	Password   string `gorm:"size:32"`
	Permission uint
}

type Device struct {
	gorm.Model `json:"-"`
	Name       string `gorm:"size:32;unique:true" json:"name"`
	Type       string `gorm:"size:32" json:"type"`
	Icon       string `gorm:"size:64" json:"icon"`
	Online     uint   `json:"online"`
}

type DeviceDetails struct {
	gorm.Model
	Name     string `gorm:"size:32;unique:true"`
	Type     string `gorm:"size:32"`
	Icon     string `gorm:"size:64"`
	Value    string
	Unit     string `gorm:"size:16"`
	Size     string `gorm:"size:16"`
	DeviceID int
	Device   Device
}

var Db *gorm.DB

func db_initialize() {
	var err error
	Db, err = gorm.Open(sqlite.Open("database.db"), &gorm.Config{})
	if err != nil {
		panic("failed to connect database")
	}

	// 迁移 schema
	Db.AutoMigrate(&User{}, &Device{}, &DeviceDetails{})
	/*
		// Create
		//db.Create(&Product{Code: "D42", Price: 100})

		// Read
		var product Product
		db.First(&product, 1)                 // 根据整型主键查找
		db.First(&product, "code = ?", "D42") // 查找 code 字段值为 D42 的记录

		// Update - 将 product 的 price 更新为 200
		db.Model(&product).Update("Price", 200)
		// Update - 更新多个字段
		db.Model(&product).Updates(Product{Price: 200, Code: "F42"}) // 仅更新非零值字段
		db.Model(&product).Updates(map[string]interface{}{"Price": 200, "Code": "F42"})

		// Delete - 删除 product
		db.Delete(&product, 1)
	*/
}
