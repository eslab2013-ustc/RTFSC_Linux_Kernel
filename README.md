##为什么转到github上##
2012级读内核使用 [Google Group](https://groups.google.com) 来通知、讨论、上传文件的，但效果不是很好，原因是多方面的，这次我们尝试使用[GitHub](https://github.com)来管理，Github 相比 GG 有很多自身的优势：  

* 不用翻墙，虽然有时访问也会受限，但大多时间还是能正常访问的；
* 能对代码、文档进行版本控制，个人认为这是最大的优势；
* Github 的 Issue 模块可以用来跟踪阅读内核所遇到的问题；
* Github 其他功能模块，如Wiki、Page、Gist等等有功能强大且方便的用途；
* Github 中托管的开源代码数量已经超过[Google Code](https://code.google.com)、[Source Forge](http://sourceforge.net/)，再不用Github你就 **OUT** 了！
* ...

当然Github也有其劣势，首先上手还是有点困难的，你得学点 Git 的基本命令、你得了解 Github 使用方法、你还需要懂得如何使用 Markdown 语法来编辑等等，但是，这些总的来都比较简单，而且作为一个搞技术的，这些是必须要掌握的。   
使用Github 只是我个人的想法，希望用过之后能得到大家的认同，我接触 Git 和 Github 有段时间了，但用的很少，因此也希望趁这机会和大家一起学习学习，成为一个真正的 Gittor 和 Githubber !

##使用方法
我在Github 上建立一个 Organization，名叫 **eslab2013-ustc**，大家可以通过 [https://github.com/eslab2013-ustc](https://github.com/eslab2013-ustc)来访问，同时在这个帐号下创建了一个新的资源库 **RTFSC_Linux_Kernel**，即为你们读Linux 内核所有资料所存放的资源库。  
那么我们如何来协作管理该资源库呢？  

* 注册帐号，fork 资源库  
每个人首先要注册一个github 的帐号，登陆之后进入资源库 [eslab2013-ustc/RTFSC_Linux_Kernel](https://github.com/eslab2013-ustc/RTFSC_Linux_Kernel)，右上角有个`Fork` 按钮，即将该资源库拷贝到自己的账户下。
* 安装客户端，clone 资源库   
该操作主要是将自己账户下（以下使用 account 做为该账号）的资源拷贝到本地。
	* Windows 平台   
	Windows 下使用git有命令行和图形终端两种，操作也较简单
	* Linux 平台  
	Linux下较简单，只需安装git，使用命令 `git-clone`
	* Mac OSX 平台  
	本丝没用过该系统，故略去...
* 本地与远程资源同步   
	此处远程资源指的是fork到 account 帐号下的资源库，本地是指clone到本地的资源库。在本地通过`pull` 与 `push` 操作就可以保证同步。
* 远程资源库与源资源库同步  
	源资源库指的是 eslab2013-ustc 帐号下的资源库，通过`pull request` 可保证同步。


##RTFSC_linux_kernel 管理规范
为了规范化管理，对该资源库进行如下约定，望大家遵守！
  
* `code` 目录树如下：

		/  
		|---2013.9.9_ProcessManager1  
		|		|
		|		|---topic&task.doc
		|		|
		|       |---余奇
		|       |    |---汇报主题.ppt
		|       |    |---笔记1.doc
		|       |    |---笔记2.doc
		|		|	 |---code.c
		|		|	 |---...
		|		|	 
		|       |---赵勇
		|       |    |---汇报主题.ppt
		|       |    |---笔记1.doc
		|       |    |---笔记2.doc
		|		|	 |---code.c
		|		|	 |---...	
		|		|
		|		|---讨论记录.doc
		|
		|---2013.9.16_ProcessManager2
		|		|
		|  		|---
		|		...
		| 
		|---README.md

	* README.md  介绍
	* 每次讨论课以 `日期_主题` 建立一个文件夹
		* 由刘杰及组长给每个人分配本周的阅读内核的任务，提前建立文件 `topic&task.doc`
		* 各自（汇报人必须或者非汇报人若有要共享的）以 `姓名` 建一个子文件夹，最好能提前报告一天提交，内容包括汇报的PPT、平时读内核所做的笔记、所写的代码等等
		* 讨论课结束之后，将本次讨论的记录以 `讨论记录.doc` 文件存放提交	
			

* `Issues` 管理  
github 中的 Issue 功能原本是一个轻量级的缺陷跟踪模块，在此我们充分利用上 Issue 的功能：  
首先是缺陷跟踪功能。我们在读内核中或者在作报告中经常会遇到问题，有些通过查资料或者大家讨论很快就能解决的，但更多的疑问一时半会都无法得出一个满意的答案，按照我的经验这些问题时间久了就丢掉了，但疑问仍然在那里，这不是读内核的初衷！所以，我们需要持续跟踪这些问题，直到解决！那么我们就可以在此处发表新的 Issue ，将问题描述清楚，为了能更好的将问题归类，请大家给自己的问题打上标签（Labels），最少一个、最多四个。大家可以将自己对该问题的想法写在该 Issue 的评论中，若最终有比较满意的答案，就可以 close 该 Issue。  
其次，我们还可以将 Issue 作为实验室读内核的博客，将 Issue 作为博客来使用还只是个另类的尝试，之所以选择这个，一来我们内核讨论的圈子就集中在 github 上，没有必要再去其它博客类网站再建一个公共博客，分散大家注意力；二来，github Issue 天然具有博客的特点，使用github markdown 进行编辑文本，排版轻盈、代码高亮，且具有评论、标签等功能。为了区分博客与问题，希望大家在博客分享加上"blog" 标签。


* `wiki` 管理   
github 提供了wiki 模块，方便项目团队维护项目文档，暂时这个模块我们不用。


##教程##
###git 版本控制简介


###github 基本功能


###markdown 编辑文本


---
最后将Linus 的一句名言送给大家：
>Read The F*cking Source Code!  
>　　　　　　　　　　　　　　　　——Linus
