# lab0实验报告

**黄雨石20376156**

[TOC]



## 实验思考题

### Thinking0.1

• 使用 cat Modified.txt ，观察它和第一次 add 之前的 status 一样吗，思考一 下为什么？(只有这个问题需要写到课后的实验报告中)

**answer:不一样，第一次add前为**

```
Untracked files:
  (use "git add <file>..." to include in what will be committed)
        README.txt

nothing added to commit but untracked files present (use "git add" to track)

```

**而cat Modified.txt中是**

```
Your branch is ahead of 'origin/main' by 1 commit.
  (use "git push" to publish your local commits)

Changes not staged for commit:
  (use "git add <file>..." to update what will be committed)
  (use "git restore <file>..." to discard changes in working directory)
        modified:   README.txt

no changes added to commit (use "git add" and/or "git commit -a")

```

**原因：第一次add前，该文件还未被加入版本库，因此git提示要先加入缓冲区，之后再加入版本库，而Modified.txt中，则是由于该文件再被加入版本库后，在工作区又发生了修改，因为已经加入到版本库保存了，因次提示有两种操作，第一种就是像之前那样add再commit将新的修改提交到版本库，第二种就是用restore撤销此次修改，回到上次commit状态**

### Thinking 0.2 

* 仔细看看这张图，思考一下箭头中的 add the file 、stage the file 和 commit 分别对应的是 Git 里的哪些命令呢？

  ![image-20220316235707324](C:\Users\hys\AppData\Roaming\Typora\typora-user-images\image-20220316235707324.png)

  **answer:分别对应``git commit -a <file_name> `` ，``git add <file_name>``，``git commit <file_name>``**

### Thinking 0.3

*  (1) 深夜，小明在做操作系统实验。困意一阵阵袭来，小明睡倒在了键 盘上。等到小明早上醒来的时候，他惊恐地发现，他把一个重要的代码文件 printf.c 删除掉了。苦恼的小明向你求助，你该怎样帮他把代码文件恢复呢？

  **answer:``git checkout -- printf.c``**

*  (2) 正在小明苦恼的时候，小红主动请缨帮小明解决问题。小红很爽快地在键盘上 敲下了 git rm printf.c，这下事情更复杂了，现在你又该如何处理才能弥补小红的过 错呢？ 

  **answer:``git reset --hard HEAD printf.c``**

* (3) 处理完代码文件，你正打算去找小明说他的文件已经恢复了，但突然发现小明 的仓库里有一个叫 Tucao.txt，你好奇地打开一看，发现是吐槽操作系统实验的， 且该文件已经被添加到暂存区了，面对这样的情况，你该如何设置才能使 Tucao.txt 在不从工作区删除的情况下不会被 git commit 指令提交到版本库？

  **answer:``git rm --cached Tucao.txt``或者``git reset``也可以**

### Thinking 0.4

* •找到在/home/20xxxxxx/learnGit 下刚刚创建的 README.txt，没有的话就 新建一个。

   • 在文件里加入 Testing 1，add，commit，提交说明写 1。

   • 模仿上述做法，把 1 分别改为 2 和 3，再提交两次。 

  • 使用 git log 命令查看一下提交日志，看是否已经有三次提交了？记下提交说 明为 3 的哈希值a。

  • 开动时光机！使用 git reset --hard HEAD^ ，现在再使用 git log，看看什么没 了？ 

  • 找到提交说明为 1 的哈希值，使用 git reset --hard  ，再使用 git log，看看什么没了？ 

  • 现在我们已经回到过去了，为了再次回到未来，使用 git reset --hard  ，再使用 git log，我胡汉三又回来了！

  • 这一部分在课后的思考题中简单写一写你的理解即可，毕竟能够进行版本的 恢复是使用 git 很重要的一个原因。

  **answer:使用 ``git reset --hard HEAD^``再使用``git log``提交说明3没有了，使用 ``git reset --hard <Hash-code>``再使用``git log``提交2的说明没有了，其实就是``git reset --hard <version>`` 使当前的缓冲区以及工作区一起回退，并且版本库``HEAD``所指的``commit``变成了选定版本，因此``git log``的时候只可以看到选定版本的日志，选定版本之后的日志都没了**

### Thinking 0.5

* 思考下面四个描述，你觉得哪些正确，哪些错误，请给出你参考的资 料或实验证据。 

  1. 克隆时所有分支均被克隆，但只有 HEAD 指向的分支被检出。 
  2.  克隆出的工作区中执行 git log、git status、git checkout、git commit 等操作 不会去访问远程版本库。 
  3.  克隆时只有远程版本库 HEAD 指向的分支被克隆。 
  4.  克隆后工作区的默认分支处于 master 分支。

  **answer:1. 正确，3.错误，因为从os_gitlab上clone下来的仓库，在本地只要checkout就可以用了，证明分支均被克隆，只有HEAD被检出。2.正确，因为本地需要push才能把更改推送到远程。4.不正确，得看远程default分支设置是什么，一般第一个传入云端的分支是defaut分支**

### Thinking 0.6

* 执行如下命令, 并查看结果 

  • echo first 

  • echo second > output.txt 

  • echo third > output.txt 

  • echo forth >> output.txt

  **answer:1.``echo first``是往控制台输出first，``echo second > output.txt``是往output.txt输出second，``echo third > output.txt``是往output.txt输出third且覆盖之前的second，``echo forth >> output.txt``是往output.txt输出forth且是追加到third的下一行 **

### Thinking 0.7 

* 使用你知道的方法（包括重定向）创建下图内容的文件（文件命名为 test），将创建该文件的命令序列保存在 command 文件中，并将 test 文件作为批处 理文件运行，将运行结果输出至 result 文件中。给出 command 文件和 result 文件 的内容，并对最后的结果进行解释说明（可以从 test 文件的内容入手）. 具体实现 的过程中思考下列问题: ``echo echo Shell Start`` 与`` echo ‘echo Shell Start’`` 效果是否 有区别; ``echo echo $c>file1`` 与`` echo ‘echo $c>file1’`` 效果是否有区别.

  ![image-20220317190147904](C:\Users\hys\AppData\Roaming\Typora\typora-user-images\image-20220317190147904.png)

  **answer:command文件内容如下（[enter],[esc]表示键盘上对应按键）：**

  ```
  vim command [enter]
  i
  echo Shell Start ...
  echo set a = 1
  a=1
  echo set b = 2
  b=2
  echo set c = a+b
  c=$[$a+$b]
  echo c = $c
  echo save c to ./file1
  echo $c>file1
  echo save b to ./file2
  echo $b>file2
  echo save a to ./file3
  echo $a>file3
  echo save file1 file2 file3 to file4
  cat file1>file4
  cat file2>>file4
  cat file3>>file4
  echo save file4 to ./result
  cat file4>>result
  [esc]
  :wq
  [enter]
  ```

  **result文件中的内容为：**

  ```
  3
  2
  1
  ```

  **解释说明：`echo sth`，将`sth`输出至主屏幕,`echo sth > out`，将`sth`输出至`out`文件`a=1`，表示新建变量并赋值为``1``,``c=$[$a+$b]``，表示新建变量`c`，赋值为``a+b``，`cat file`表示将file的内容输出到屏幕上,`cat file>out`，表示将`file`的内容输出到`out`文件中，`out`将被`file`中内容覆盖，`cat file>>out`，表示将`file`中的内容追加到`out`文件中**

  **思考问题：`echo Shell Start`和`echo 'Shell Start'`在效果上无区别，`echo $c>file1`表示将变量``c``输出到file1中，`echo '$c>file1'`直接将`''`中的内容输出到屏幕上，即，使用单引号，不转义**

## 实验难点图示

我认为在学习lab0中最困难的其实是关于``git reset``和``git checkout``的学习过程，具体如下：

### 一些应用

将缓冲区恢复到当前版本库指向的版本即默认参数(``--mixed``)：

```
git reset <file>
```

将工作区与缓冲区都恢复到当前版本库指向的版本：

```
git reset --hard HEAD <file>(或者是<commit_id>的一部分也可以)
```

上一个版本是``^HEAD``

丢弃工作区的修改（让它回到最近一次``git commit``或者``git add``状态）比如工作区一不小心删了：

```
git checkout <file>
```

### git reset

``git reset``实际上有3个步骤，根据不同的参数可以决定执行到哪个步骤(`--soft`, `--mixed`, `--hard`)。

1. 改变``HEAD``所指向的``commit``(`--soft`)
2. 执行第1步，将``Index``区域更新为``HEAD``所指向的``commit``里包含的内容(`--mixed`)
3. 执行第1、2步，将``Working Directory``区域更新为``HEAD``所指向的``commit``里包含的内容(`--hard`)

上面讲到的``git reset``实际上不带参数的，如果带上文件参数，那么效果会是怎样的？

1. ``HEAD``不会动
2. 将那个``commit``的``snapshot``里的那个文件放到Index区域中

需要注意的是带文件参数的``git reset``没有``--hard``,`` --soft``这两个参数。只有``--mixed``参数。

### git checkout

前面讲到``checkout``是会修改``HEAD``的指向，变更``Index``区域里的内容，修改``Working Directory``里的内容。

这看上去很像`reset --hard`，但和`reset --hard`相比有两个重要的差别

1. ``reset``会把``working directory``里的所有内容都更新掉
2. ``checkout``不会去修改你在``Working Directory``里修改过的文件
3. ``reset``把``branch``移动到``HEAD``指向的地方(``HEAD``本身也移动，相当于``HEAD``和``branch``一起移动)
4. ``checkout``则把``HEAD``移动到另一个分支

第二个区别可能有点难以理解，举例来说：假设你有两个分支``master``和``develop``，这两个分支指向不一样的``commit``，我们现在在``develop``分支上（``HEAD``指向的地方）

如果我们`git reset master`，那么``develop``就会指向``master``所指向的那个``commit``。

如果我们`git checkout master`，那么``develop``不会动，只有``HEAD``会移动。``HEAD``会指向``master``。看图：

![图片描述](https://segmentfault.com/img/bVz7pB?w=500&h=366)

## 体会与感想

我认为这次lab0课下其实是有一定难度的，虽然之前就接触``git``，``gcc``不过对于``shell``脚本以及``linux``的进阶命令``grep``，``awk``，``sed``学的还是十分艰难，做lab0加知识学习花了大概``4``，``5``个钟左右，特别是写``shell``和``Makefile``的时候在几种写法种反复横跳，非常难受，又比如对于``linux``命令中**双引号，单引号，反引号，不加引号**的用法我之前也挺迷惑的。最后想法和体会就是要小心注意别敲错命令，要好好学习``vim``和``linux``的一些便捷操作，看了室友的一些骚操作，我大为震撼。

## 指导书反馈

* 建议对于``sed``，``grep``，``awk``等等的应用再加一点例子，不然只放寥寥几个例子，感觉帮助不大
* 建议增加关于``linux``命令中双引号，单引号，反引号，不加引号的区别以及用法的讲解，个人感觉非常有用