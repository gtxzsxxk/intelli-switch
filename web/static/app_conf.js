function CanvasInit(vs_container, myCanvas) {
    var scr_width = vs_container.offsetWidth;
    var scr_height = scr_width / screenWidth * screenHeight;
    virtualPixelSize = scr_width / screenWidth;
    console.log(virtualPixelSize);
    CanvasWidth = scr_width;
    CanvasHeight = scr_height;
    vs_container.style.height = scr_height.toFixed(2).toString() + 'px';
    myCanvas.setAttribute("width", CanvasWidth);
    myCanvas.setAttribute("height", CanvasHeight);
    canvas = myCanvas;
    ctx = canvas.getContext("2d");
    CanvasTest();
}

function CanvasTest() {
    ctx.fillStyle = "#808000";
    ctx.fillRect(0, 0, CanvasWidth, CanvasHeight);
    ctx.font = CanvasHeight / guisize_y + "px Courier New";
    ctx.fillStyle = "#000000";
    //ctx.fillText("Hello World!34",10,CanvasHeight/guisize_y); 
    printf("  IntelliSw");
    printf("Mon 15:43:11");
    printf("Temp:27.56'C");
    printf("QNH:1013.13hPa");
    printf("ILLU:103lx");
    printf("HUMI:56 per");
}

function SetAppActive(id) {
    alert(id + "已激活");
}

function PixelCalc(a) {
    return parseInt(a) + 0.5;
}

function SetColor(color) {
    ctx.fillStyle = color;
    ctx.strokeStyle = color;
    ctx.shadowColor = color;
}

function DrawSingleWord(word, x, y) {
    var modelArray = chars_pixel_jsondata[word];
    SetColor("#000000")
    for (var b = 0; b < 4; b++) {
        var loc_x = x;
        for (var i = 0; i < modelArray.length; i++) {
            var currentData = modelArray[i];
            var loc_y = y;
            for (var j = 0; j < 8; j++) {
                if ((currentData & 1) == 1)
                    SetColor("#000000")
                else
                    SetColor("#808000")
                ctx.fillRect(loc_x, loc_y, virtualPixelSize, virtualPixelSize);
                loc_y += virtualPixelSize;
                currentData = currentData >> 1;
            }
            loc_x += virtualPixelSize;
        }
    }
}

var currentLine = 0;

function MoveLine() {
    var toreturn = currentLine++;
    if (currentLine == guisize_y)
        currentLine = 0;
    return toreturn;
}

var ScreenDataArray=new Array(guisize_y);

function DrawCursor()
{
    document.getElementById('line_cursor').style.marginTop=currentLine*virtualPixelSize * word_y+"px";
}

function printf(str) {
    var toshowStr = str;
    while (toshowStr.length > guisize_x) {
        var line = MoveLine();
        ScreenDataArray[line]="";
        for (var i = 0; i < guisize_x; i++) {
            DrawSingleWord(toshowStr[i], i * virtualPixelSize * word_x, line * virtualPixelSize * word_y);
            ScreenDataArray[line]+=toshowStr[i];
        }
        toshowStr = toshowStr.slice(14, toshowStr.length);
    }
    var line = MoveLine();
    ScreenDataArray[line]="";
    for (var i = 0; i < guisize_x; i++) {
        if(i<toshowStr.length)
        {
            DrawSingleWord(toshowStr[i], i * virtualPixelSize * word_x, line * virtualPixelSize * word_y);
            ScreenDataArray[line]+=toshowStr[i];
        }
        else
        {
            ScreenDataArray[line]+=' ';
        }
    }
    DrawCursor();
}

function l_print(str) {
    var line = MoveLine();
    ScreenDataArray[line]="";
    for (var i = 0; i < guisize_x; i++) {
        if(i<str.length)
        {
            DrawSingleWord(str[i], i * virtualPixelSize * word_x, line * virtualPixelSize * word_y);
            ScreenDataArray[line]+=str[i];
        }
        else
        {
            ScreenDataArray[line]+=' ';
            DrawSingleWord(" ", i * virtualPixelSize * word_x, line * virtualPixelSize * word_y);
        }
    }

    DrawCursor();
}


function ExportGUI()
{
    var output="";
    for(var i in ScreenDataArray)
        output+=ScreenDataArray[i];
    document.getElementById('guiresult').innerHTML="#字符数据总长度："+output.length+"&#10;";
    if(output.length!=guisize_x*guisize_y)
        document.getElementById('guiresult').innerHTML+="#错误：字符数据总长度无法匹配："+"&#10;";
    document.getElementById('guiresult').innerHTML+="#Python变量：&#10;guiStr:str=\""+output+"\"";
    hljs.highlightBlock(document.getElementById('guiresult'));
}

function ClearGUI()
{
    var output="";
    currentLine=0;
    for(var i=0;i<guisize_x*guisize_y;i++)
        output+=" ";
    printf(output);
}

function ClearCurrentLine()
{
    for(var i=0;i<guisize_x;i++)
    {
        DrawSingleWord(' ', i * virtualPixelSize * word_x, currentLine * virtualPixelSize * word_y);
    }
}

function CursorUp()
{
    currentLine--;
    if(currentLine<0)
        currentLine=guisize_y-1;
    DrawCursor();
}

function CursorDown()
{
    currentLine++;
    if(currentLine==guisize_y)
        currentLine=0;
    DrawCursor();
}