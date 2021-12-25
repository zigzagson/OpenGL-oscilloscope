 电子科技大学测控19级攀登计划
===============================
项目一：基于多维映射的瞬态信号捕获与分析——上位机实现
-----------------------------------

本项目基于OpenGL实现示波器界面，使用socket与下位机FPGA进行UDP通信
FPGA获取数据发送到上位机显示波形。

## 操作：
* 左键拖动图像
* 右键
    * 短按暂停
    * 长按autoset
* 滚轮
    * 在窗口主体滚轮放大图像
    * 在窗口右边滚轮单独调整纵轴
    * 在窗口底部滚轮单独调整横轴
* 图标
    * 左键点击图标autoset
    * 右键点击图标开/关触发
    * 中间点击图标重新读取init.json

### init.json:
> rate: 采样率（单位MHz）
> 
> color: 颜色，RGB[A]，范围0~1
> 
> auto: autoset时，纵轴显示多少格，横轴显示几个周期
> 
> scroll: 滚轮敏感度
> 
> debug: debug时模拟正弦波的峰峰值(mv)，直流偏置(mv)，周期(us)