# 疯狂电路完赛

#### 介绍
缩微赛道疯狂电路
本代码采用分区大津法（三区自适应阈值）应对光照不均，扫线使用逐飞标准三向扫描提取中线。拐点通过列坐标跳变检测，多拐点筛选去干扰后由矩形框按需激活做二次验证。
状态机根据拐点类型查表执行中线偏置，将偏差输出给方向环PID控制转向。退出条件分场景：直行靠编码器距离，转弯靠陀螺角度回正。
目前最大问题仍是光线适应性不足。

#### 软件架构
软件架构说明


#### 安装教程

1.  xxxx
2.  xxxx
3.  xxxx

#### 使用说明

1.  xxxx
2.  xxxx
3.  xxxx

#### 参与贡献

1.  Fork 本仓库
2.  新建 Feat_xxx 分支
3.  提交代码
4.  新建 Pull Request


#### 特技

1.  使用 Readme\_XXX.md 来支持不同的语言，例如 Readme\_en.md, Readme\_zh.md
2.  Gitee 官方博客 [blog.gitee.com](https://blog.gitee.com)
3.  你可以 [https://gitee.com/explore](https://gitee.com/explore) 这个地址来了解 Gitee 上的优秀开源项目
4.  [GVP](https://gitee.com/gvp) 全称是 Gitee 最有价值开源项目，是综合评定出的优秀开源项目
5.  Gitee 官方提供的使用手册 [https://gitee.com/help](https://gitee.com/help)
6.  Gitee 封面人物是一档用来展示 Gitee 会员风采的栏目 [https://gitee.com/gitee-stars/](https://gitee.com/gitee-stars/)
