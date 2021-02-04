function ClockEvent()
{
    $('#reftime').html((parseInt($('#reftime').html())+1).toString());
    if(parseInt($('#reftime').html())>=60&&parseInt($('#reftime').html())<=70)
        GetView();
}

function GetView() {
    $.ajax({
        url: "view/",
        type: "GET",
        success: function (result) {
            var jsonObj = JSON.parse(result);
            for (var k in jsonObj) {
                $('#' + k).html(jsonObj[k]);
            }
            $(".gif").css("display", "none");
        }
    });
    
}

var isToday = true;
function ChangeDate() {
    var time = new Date();
    var day = ("0" + time.getDate()).slice(-2);
    var month = ("0" + (time.getMonth() + 1)).slice(-2);
    var today = time.getFullYear() + "-" + (month) + "-" + (day);
    if ($('#date_picker').val() == today)
        isToday = true;
    else
        isToday = false;
    GetTempData();
    GetPressData();
    GetIlluData();
}
var temp_ctx = null;
var tempChart = null;

function GetTempData() {
    $.ajax({
        url: 'temp/',
        type: 'GET',
        data: { 'from': $('#date_picker').val() },
        success: function (result) {
            var mydata = JSON.parse(result);
            //这里如果遇到0度，会直接识别为没有数据，我找不到更好的解决方法
            var min_value = 99999; var max_value = -99999;
            for (var i = 0; i < mydata.length; i++) {
                mydata[i] = parseFloat(mydata[i].toFixed(2));
                if (mydata[i] > max_value)
                    max_value = mydata[i];
                if (mydata[i] < min_value)
                    min_value = mydata[i];

                var now = new Date();
                if (i > now.getHours() && isToday)
                    mydata[i] = null;
            }
            temp_ctx = document.getElementById("tempChart").getContext('2d');
            tempChart = new Chart(temp_ctx, {
                type: 'line',
                data: {
                    labels: ["0时", "1时", "2时", "3时", "4时", "5时", "6时", "7时", "8时", "9时", "10时"
                        , "11时", "12时", "13时", "14时", "15时", "16时", "17时", "18时", "19时", "20时", "21时", "22时", "23时"],
                    datasets: [{
                        label: '温度变化曲线',
                        data: mydata,
                        backgroundColor: [
                            'rgba(255, 99, 132, 0.2)'
                        ],
                        borderColor: [
                            'rgba(255,99,132,1)'
                        ],
                        borderWidth: 1
                    }]
                },
                options: {
                    spanGaps: true,
                    scales: {
                        y: {
                            suggestedMin: min_value - 2,
                            suggestedMax: max_value + 2
                        }
                    }
                }
            });
        }
    });
}

var press_ctx = null;
var pressChart = null;
function GetPressData() {
    $.ajax({
        url: 'press/',
        type: 'GET',
        data: { 'from': $('#date_picker').val() },
        success: function (result) {
            var mydata = JSON.parse(result);
            for (var i = 0; i < mydata.length; i++) {
                mydata[i] = parseFloat(mydata[i].toFixed(2));
                var now = new Date();
                if (i > now.getHours() && isToday)
                    mydata[i] = null;
            }
            press_ctx = document.getElementById("pressChart").getContext('2d');
            pressChart = new Chart(press_ctx, {
                type: 'line',
                data: {
                    labels: ["0时", "1时", "2时", "3时", "4时", "5时", "6时", "7时", "8时", "9时", "10时"
                        , "11时", "12时", "13时", "14时", "15时", "16时", "17时", "18时", "19时", "20时", "21时", "22时", "23时"],
                    datasets: [{
                        label: '气压变化曲线',
                        data: mydata,
                        backgroundColor: [
                            'rgba(255, 99, 132, 0.2)'
                        ],
                        borderColor: [
                            'rgba(255,99,132,1)'
                        ],
                        borderWidth: 1
                    }]
                },
                options: {
                    spanGaps: true,
                    scales: {
                        y: {
                            suggestedMin: 950,
                            suggestedMax: 1060
                        }
                    }
                }
            });
        }
    });
}

var illu_ctx = null;
var illuChart = null;
function GetIlluData() {
    $.ajax({
        url: 'illu/',
        type: 'GET',
        data: { 'from': $('#date_picker').val() },
        success: function (result) {
            var mydata = JSON.parse(result);
            for (var i = 0; i < mydata.length; i++) {
                //if (mydata[i] == 0)
                mydata[i] = parseFloat(mydata[i].toFixed(2));
                var now = new Date();
                if (i > now.getHours() && isToday)
                    mydata[i] = null;
            }
            illu_ctx = document.getElementById("illuChart").getContext('2d');
            illuChart = new Chart(illu_ctx, {
                type: 'line',
                data: {
                    labels: ["0时", "1时", "2时", "3时", "4时", "5时", "6时", "7时", "8时", "9时", "10时"
                        , "11时", "12时", "13时", "14时", "15时", "16时", "17时", "18时", "19时", "20时", "21时", "22时", "23时"],
                    datasets: [{
                        label: '光照强度变化曲线',
                        data: mydata,
                        backgroundColor: [
                            'rgba(255, 99, 132, 0.2)'
                        ],
                        borderColor: [
                            'rgba(255,99,132,1)'
                        ],
                        borderWidth: 1
                    }]
                },
                options: {
                    spanGaps: true,
                    scales: {
                        yAxes: [{
                            ticks: {
                            }
                        }]
                    }
                }
            });
        }
    });
}

function OnTimeEvent() {
    RefreshHTML();
}

function navStick(liid, target) {
    document.getElementById('li1').setAttribute('class', '');
    document.getElementById('li2').setAttribute('class', '');
    document.getElementById('li3').setAttribute('class', '');
    document.getElementById('li4').setAttribute('class', '');
    document.getElementById(liid).setAttribute('class', 'active');
    $('html,body').animate({ scrollTop: $('#' + target).offset().top - 70 }, 1000);

}

function Talert(msg) {
    alert(msg);
    if (IsClient())
        window.web.ToastAlert(msg);
}

function isContains(str, substr) {
    return str.indexOf(substr) >= 0;
}

function debug() {
    //debuginfo
    Talert(navigator.userAgent);
}

function IsClient() {
    return isContains(navigator.userAgent, "app/");
}

function debuginfo() {
    var toshowinfo = "当前平台：";
    if (isContains(navigator.userAgent, "app/"))
        toshowinfo += "安卓客户端";
    else if (isContains(navigator.userAgent.toLowerCase(), "android"))
        toshowinfo += "安卓";
    else if (isContains(navigator.userAgent.toLowerCase(), "windows"))
        toshowinfo += "Windows";
    else if (isContains(navigator.userAgent.toLowerCase(), "linux"))
        toshowinfo += "Linux";
    else
        toshowinfo += "Invalid Platform"
    $('#serverinfodiv').html($('#serverinfodiv').html() + "<br />" + toshowinfo);
}

function webTerminal() {
    $("#termin-exec").attr({ "disabled": "disabled" });
    if ($('#commandbox').val() == "") {
        Talert('请输入有效命令');
        $("#termin-exec").attr({ "disabled": "" });
        return;
    }
    $.ajax({
        url: 'command/',
        type: 'GET',
        data: { 'c': $('#commandbox').val() },
        success: function (result) {
            Talert(result);
            GetView();
        }
    });
    $("#termin-exec").attr({ "disabled": "" });
}

function RefreshHTML() {
    if (IsClient())
        window.web.RefreshPage();
    else
        window.location.reload();
}

function exitPAGE() {
    if (IsClient())
        window.web.ExitProgram();
    else {
        window.opener = null;
        window.open('', '_self');
        window.close();
    }
}

function device_ctl(deviceId, cmd) {
    $("#devbtn" + deviceId.toString()).attr({ "disabled": "disabled" });
    $.ajax({
        url: 'command/',
        type: 'GET',
        data: { 'c': 'devedit ' + deviceId.toString() + ' ' + cmd },
        success: function (result) {
            if (isContains(result, 'errs')) {
                Talert(result.split('\r\n')[1]);
            }
            /*else {
                $('#devbtn' + deviceId.toString()).val(result.split('\r\n')[1]);
                if (result.split('\r\n')[1] == 'On') {
                    $('#devbtn' + deviceId.toString()).removeClass('btn-primary');
                    $('#devbtn' + deviceId.toString()).addClass('btn-danger');
                } else {
                    $('#devbtn' + deviceId.toString()).addClass('btn-primary');
                    $('#devbtn' + deviceId.toString()).removeClass('btn-danger');
                }

            }*/
            GetView();
            //$('#devbtn'+deviceId.toString()).val(result);
        }
    });
    $("#devbtn" + deviceId.toString()).attr({ "disabled": "" });
}