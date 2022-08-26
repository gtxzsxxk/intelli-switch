## 设备属性：
#### 类型type
- display：用于显示value。图标由icon决定，单位由unit控制，若设置为小块，则在size中写入single。若为大块，置空size
- control：用来显示设备状态并提供控制功能。若要实现控制功能，则需要服务器与设备建立长连接，保证双向数据通信。icon、size在此处将不会生效。对于unit：
    - toggle：开关类设备。value应为boolean类型，使用string存储
    - range(a,b)：值类设备。value为[a,b]中的值，使用string存储
    - exec：扳机类设备。value无意义。激活exec后，在设备上对应事件将被执行一次。
- script：用于在server端执行其它脚本。value对应该脚本的激活状态。unit应为toggle
- chart：unit为y轴单位。value:{xAxis:[...],yAxis:[xxx],type:'line...'}。存储value需要stringify
- map：存储地图数据。value:[[x1,y1],[x2,y2],...]。存储value需要stringify

## 用户权限说明：
- level-0：仅浏览数据
- level-1：允许上传数据，不允许编辑设备
- level-2：允许上传数据，允许编辑设备