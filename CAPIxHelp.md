<div id="table-of-contents">
<h2>Table of Contents</h2>
<div id="text-table-of-contents">
<ul>
<li><a href="#sec-1">1. 简介</a></li>
<li><a href="#sec-2">2. 标识符</a></li>
<li><a href="#sec-3">3. 基本命令</a>
<ul>
<li><a href="#sec-3-1">3.1. Mem</a>
<ul>
<li><a href="#sec-3-1-1">3.1.1. Alloc</a></li>
<li><a href="#sec-3-1-2">3.1.2. Free</a></li>
<li><a href="#sec-3-1-3">3.1.3. Put</a></li>
<li><a href="#sec-3-1-4">3.1.4. Print</a></li>
<li><a href="#sec-3-1-5">3.1.5. Copy</a></li>
<li><a href="#sec-3-1-6">3.1.6. 用途</a></li>
</ul>
</li>
<li><a href="#sec-3-2">3.2. API</a>
<ul>
<li><a href="#sec-3-2-1">3.2.1. Call</a></li>
<li><a href="#sec-3-2-2">3.2.2. Exec</a></li>
<li><a href="#sec-3-2-3">3.2.3. 注</a></li>
</ul>
</li>
<li><a href="#sec-3-3">3.3. CAPIDll</a></li>
<li><a href="#sec-3-4">3.4. Var</a>
<ul>
<li><a href="#sec-3-4-1">3.4.1. SetCall</a></li>
<li><a href="#sec-3-4-2">3.4.2. GetCall</a></li>
</ul>
</li>
</ul>
</li>
<li><a href="#sec-4">4. 实例</a>
<ul>
<li><a href="#sec-4-1">4.1. Mem Alloc Free</a></li>
<li><a href="#sec-4-2">4.2. Mem Put</a></li>
<li><a href="#sec-4-3">4.3. Mem Copy</a></li>
<li><a href="#sec-4-4">4.4. Mem Print</a></li>
<li><a href="#sec-4-5">4.5. API Call</a></li>
<li><a href="#sec-4-6">4.6. API Exec</a></li>
<li><a href="#sec-4-7">4.7. GetCall</a></li>
</ul>
</li>
<li><a href="#sec-5">5. CAPIx相对CAPI有哪些改进</a></li>
<li><a href="#sec-6">6. 等待加入的功能</a></li>
</ul>
</div>
</div>


# 简介<a id="sec-1" name="sec-1"></a>

致敬D大和他的CAPI(注入版)

简介:

CAPI是bathome的defanvie开发的一款第三方,堪称批处理第三方的登峰造极之作

CAPI提供的最大功能就是一个:调用系统API 除此之外,CAPI并没有任何功能

而且其注入式的特点,一方面使得其调用速度得到了飞一般的提升,另一方面也使得其能够保存进程数据

这两个功能,赋予了CAPI无穷的力量

何为系统API?简而言之,是MS为了便利开发者,而封装在dll中的一系列函数

通过这些函数,开发者可以不需要理解系统的内部原理就方便地开发出产品

比如著名的image(第一代),就是调用了gdi32中的API从而实现了绘图功能

而利用CAPI,我们可以轻松地实现这个功能,而且由于CAPI是注入式的,我们可以甚至可以实现双缓冲

即先把图片画到内存中的画布上,等待所有的图片都绘制完成,再将整块画布复制到CMD中

这样做的最大好处就是可以避免闪烁,实现流畅地绘制

然而CAPI有几个问题,导致了它虽然强大,但却很少被使用

1.采取远程线程注入的方法将dll注入到cmd中,这是病毒的常用伎俩,因此误杀率极高

2.开发者defanvie已经很久没有现身,而且CAPI没有开源.已经无法在最新的系统上运行了

CAPIx就是为了解决这两个问题而开发的

首先CAPIx摒弃了远程线程注入的方式,而是使用一个修改过的提取自win 2003的cmd

这个cmd在启动时会自动加载CAPIx.dll,免去了注入的危险

其次CAPIx的开发者是可以联系到的,而且CAPIx完全开源,这为修复bug和增加功能提供了方便

并且CAPIx在API调用的功能上做出了进一步的加强

在API Call之外又提供了一个新命令API Exec,用来调用遵循\_\_cdecl约定的函数

简而言之,可以调用其他dll中的函数,比如msvcrt中的大部分函数,像printf, scanf之类的

又比如调用regex2.dll实现正则匹配,还可以自己写dll供Exec调用,比如写一个浮点运算的dll代替set/a之类的

CAPIx潜力无限!

下载地址: <https://github.com/YinTianliang/CAPIx/tree/master/bin>

请务必下载CAPIx.dll和cmd.exe,  CAPIx\_Help.html为详细帮助,建议下载

使用CAPIx进行的程序只能在上面这个cmd.exe中正常运行

所以建议使用以下JS启动游戏(该cmd.exe需要与js, capix.dll放在同一目录下)

    1  new ActiveXObject('WScript.Shell').Run('"' + WScript.ScriptFullName.replace(/[^\\]*$/,'') + 'cmd.exe" /c 游戏主程序.bat')

或者在批处理开头加上这两行

    1  %1@start "" "%~dp0cmd.exe" "/c %~fs0 :"
    2  %1@exit

为了兼容以前的的CAPI作品, CAPIx的语法和CAPI是一样的, 只是在此基础之上做了加强, 使之更加人性化

本文档中的大部分内容来自<http://www.bathome.net/thread-19238-1-1.html>

# 标识符<a id="sec-2" name="sec-2"></a>

标识符用来添加在参数前面, 指定这个参数的类型

<table border="2" cellspacing="0" cellpadding="6" rules="groups" frame="hsides">


<colgroup>
<col  class="left" />

<col  class="left" />

<col  class="left" />
</colgroup>
<thead>
<tr>
<th scope="col" class="left">符号</th>
<th scope="col" class="left">大小</th>
<th scope="col" class="left">意义</th>
</tr>
</thead>

<tbody>
<tr>
<td class="left">.</td>
<td class="left">1B</td>
<td class="left">字节</td>
</tr>


<tr>
<td class="left">:</td>
<td class="left">2B</td>
<td class="left">短整形</td>
</tr>


<tr>
<td class="left">;</td>
<td class="left">4B</td>
<td class="left">长整形</td>
</tr>


<tr>
<td class="left">~</td>
<td class="left">4B</td>
<td class="left">浮点型</td>
</tr>


<tr>
<td class="left">\`</td>
<td class="left">8B</td>
<td class="left">双浮点型</td>
</tr>


<tr>
<td class="left">#</td>
<td class="left">-</td>
<td class="left">字符串(ANSI)</td>
</tr>


<tr>
<td class="left">$</td>
<td class="left">-</td>
<td class="left">字符串(Unicode)</td>
</tr>


<tr>
<td class="left">\*</td>
<td class="left">4B</td>
<td class="left">变量(的地址),相当于C语言中的&.可以后跟"; # $ "表示这个指针指向的变量的类型,默认为$</td>
</tr>


<tr>
<td class="left">@</td>
<td class="left">-</td>
<td class="left">移动指针,后跟欲移动的距离,可以为负</td>
</tr>
</tbody>
</table>

# 基本命令<a id="sec-3" name="sec-3"></a>

## Mem<a id="sec-3-1" name="sec-3-1"></a>

### Alloc<a id="sec-3-1-1" name="sec-3-1-1"></a>

Mem Alloc [size]
-   用途:申请一块内存
-   参数:[size]为该内存块的大小
-   返回值:该内存块的地址
-   注意:内存不用时需要释放, 避免造成内存泄漏

### Free<a id="sec-3-1-2" name="sec-3-1-2"></a>

Mem Free [addr]
-   用途:释放一块内存
-   参数:[addr]为欲释放的内存地址

### Put<a id="sec-3-1-3" name="sec-3-1-3"></a>

Mem Put [s][addr] [s][data] [s][data] &#x2026;
-   用途:将数据按格式写入内存地址中
-   参数:[s][addr]为欲写入的地址, [s]可选"; \*". [s][data]为欲写入的数据, [s]可选". : ; ~ \` # $ \* @"
-   注意:写入的数据量若超过该内存块的大小可能导致cmd崩溃

### Print<a id="sec-3-1-4" name="sec-3-1-4"></a>

Mem Print [s][addr] [s][var] [s][var] &#x2026;
-   用途:将指定地址处的内容输出到变量中
-   参数:[s][addr]为内存地址, [s]可选"; \*". [s][var]为输出参数, [s]可选". : ; ~ \` # $ \* @", [var]为变量名

### Copy<a id="sec-3-1-5" name="sec-3-1-5"></a>

Mem Copy [s][dst] [s][src] [sz]
-   用途:将源地址的一段内存复制到目标地址
-   参数:[s]可选"; \*"

### 用途<a id="sec-3-1-6" name="sec-3-1-6"></a>

Mem系列的命令提供了对内存的操作,其最大的意义就在于实现结构体或数组

使用时要注意结构体的对齐

## API<a id="sec-3-2" name="sec-3-2"></a>

### Call<a id="sec-3-2-1" name="sec-3-2-1"></a>

API Call [dll] [s][API] [s][data] [s][data] &#x2026;
-   用途:调用遵循\_\_stdcall约定的函数(一般为WINAPI)
-   参数:[dll]为dll相对路径, [API]为API函数全名, [s]为返回值类型, 默认为";", 可选"~ \`". [s][data]为参数, [s]可选"; ~ \` $ # \*"
-   返回值:该API的返回值

### Exec<a id="sec-3-2-2" name="sec-3-2-2"></a>

API Exec [dll] [s][API] [s][data] [s][data] &#x2026;
-   用途:调用遵循\_\_cdecl约定的函数(一般除WINAPI以外都是)

### 注<a id="sec-3-2-3" name="sec-3-2-3"></a>

API是可以指定返回值类型的

以上两个命令的[s][API]可以用该函数在内存中对应的地址来表示,此时[dll]应为"0"

如set CAPI=API Exec 0 16777215 ;0 ;1

## CAPIDll<a id="sec-3-3" name="sec-3-3"></a>

-   **CAPIll /?:** 返回CAPIx的基本信息
-   **CAPIDll Ver:** 返回CAPIx的版本, 保存在变量CAPI\_Ret中

## Var<a id="sec-3-4" name="sec-3-4"></a>

### SetCall<a id="sec-3-4-1" name="sec-3-4-1"></a>

-   **SetCall Enable:** 开启SetCall调用方式,即set CAPI=xxxx,返回值在变量CAPI\_Re中t
-   **SetCall Disable:** 关闭SetCall调用方式(默认开启)

### GetCall<a id="sec-3-4-2" name="sec-3-4-2"></a>

-   **GetCall Enable:** 开启GetCall调用方式,即echo %CAPI xxx%,返回值即%CAPI xxx%扩展的结果
-   **GetCall Disable:** 关闭GetCall调用方式

# 实例<a id="sec-4" name="sec-4"></a>

## Mem Alloc Free<a id="sec-4-1" name="sec-4-1"></a>

    1  @echo off
    2  set "CAPI=Mem Alloc 4"
    3  set "lpAddress=%CAPI_Ret%"
    4  echo %lpAddress%
    5  pause
    6  set "CAPI=Mem Free %lpAddress%"
    7  pause

创建了一块大小为4的内存，内存地址保存在lpAddress里  
   在批处理第一次暂停时，使用工具查看cmd.exe内存，可以看到在输出的地址处为4个空白字节的内存，第二次pause时，可以看到内存已经被释放

## Mem Put<a id="sec-4-2" name="sec-4-2"></a>

    1  @echo off
    2  set var=hello
    3  set data=0123456789
    4  set "CAPI=Mem Put *data .97 @1 :25105 #ab $ab *var"
    5  echo %data%
    6  pause

执行前data变量的内存内容为30 00 31 00 32 00 33 00 34 00 35 00 36 00 37 00 38 00 39 00  
   执行写入命令时，此时指针指向第1个字节，.97将1个字节为97写入，于是变成了61 00 31 00 &#x2026;，指针后移1位，指向第2个字节  
   @1将指针后移1位，此时指针指向第3个字节  

:25105，此时指针指向第3个字节，将2个字节为25105写入，于是变成了61 00 62 11 32 00 &#x2026;，指针后移2位  
   #ab，此时指针指向第5位，将2个字节的ANSI字符串ab写入，于是变成了61 00 62 11 61 62 33 00 34 00 &#x2026;，指针后移2位  
   $ab，此时指针指向第7位，将4个字节的Unicode字符串ab写入，于是变成了61 00 62 11 61 62 61 00 62 00 &#x2026;，指针后移4位  
   /h\*var，此时指针指向第11位，将var变量h的内容全部写入  

## Mem Copy<a id="sec-4-3" name="sec-4-3"></a>

    1  @echo off
    2  set var=hello
    3  set data=0123456789
    4  set "CAPI=Mem Copy *data *var 6"
    5  echo %data%
    6  pause

执行时，将var变量的前4个字节复制到data变量中，即是“hel”  
因此输出“hel3456789”

## Mem Print<a id="sec-4-4" name="sec-4-4"></a>

    1  @echo off
    2  set var=0123456
    3  set "CAPI=Mem Print *var .output_1 @2 :output_2 @1 #output_3 $output_4"
    4  set output_
    5  pause

var变量的内容为30 00 31 00 32 00 33 00 34 00 35 00 36 00  
.output\_1，此时指向第1个字节，将1字节的内容“31”放入output\_1变量中，也就是48   
@2，此时指针指向第2个字节，将指针后移2位，此时指针指向第4个字节  
:output\_2，此时指针指向第4个字节，将2字节的内容“00 32”放入output\_2变量中，也就是12800   
@1，此时指针指向第6个字节，将指针后移1位，此时指针指向第7个字节  
#output\_3，此时指针指向第7个字节，将接下来的内容作为ANSI字符串放入output\_3变量中，也就是“33”，字符串3   
$output\_4，此时指针指向第9个字节，将接下来的内容作为Unicode字符串放入output\_4变量中，也就是“34 00 35 00 36 00”，字符串456   

## API Call<a id="sec-4-5" name="sec-4-5"></a>

    1  @echo off
    2  set "data=message"
    3  set "CAPI=API Call user32 MessageBoxW ;0 *data $title ;1"
    4  echo %CAPI_Ret%
    5  pause

调用API MessageBox，第一个参数为0，第二个参数为data变量的地址，第三个参数为Unicode字符串title，第四个参数为1  
由于CMD内部将变量data储存为Unicode，因此应使用Unicode版本的API，也就是MessageBoxW  
(注:CAPIx的\*标识符得到了增强,可以使用\*#data来强制将data转换为ANSI字符串)  

## API Exec<a id="sec-4-6" name="sec-4-6"></a>

    1  @echo off
    2  set "data=123|456|789"
    3  set CAPI=API Exec msvcrt sscanf *#data "#%d|%^d|%d" *;_1 *;_2
    4  echo %_1%  %_2%
    5  pause

调用C语言库函数sscanf,该函数遵循cdecl调用协定,因此只能使用Exec调用  
第一个参数\*#data表示取变量data的内容, 转换为ANSI字符串, 第二个参数为sscanf的Format, 第三个和第四个参数取了两个整形变量地址  

    1  @echo off
    2  set "CAPI=API Exec msvcrt `sqrt `666"
    3  echo %CAPI_Ret%
    4  pause

调用C语言库函数sqrt,且指定返回值类型为双浮点数

    1  @echo off
    2  set "CAPI=API Exec msvcrt scanf "#%d %d" *;_1 *;_2"
    3  echo %_1%, %_2%
    4  pause

调用C语言库函数scanf, \*;\_1 的意思是取环境变量\_1的地址,将其当作整形变量传给scanf

## GetCall<a id="sec-4-7" name="sec-4-7"></a>

    1  @echo offh
    2  set CAPI=Var GetCall Enable
    3  set lpAddress=%CAPI Mem Alloc 4%
    4  echo 申请的内存地址为:%lpAddress%
    5  set CAPI=Mem Free %lpAddress%
    6  pause

# CAPIx相对CAPI有哪些改进<a id="sec-5" name="sec-5"></a>

-   "@"可以接受负值
-   参数类型中增加了浮点数和双浮点数
-   为函数返回值提供了类型
-   "\*"可以通过后跟"; # $"来指定该变量类型
-   "\*"不只是取变量内容, 而是取变量地址.API对该地址的修改会同步到变量中
-   "Exec"命令的加入
-   可以运行内存中的函数

# 等待加入的功能<a id="sec-6" name="sec-6"></a>

-   COM调用