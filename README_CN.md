以下 <TARGET_PLATFORM> 表示RK3566_RK3568或RK3588。  
 **系统是ubuntu或者Debian，板子需要接入互联网**

# rknn模型来源说明
rknn模型来源于自己训练的识别a_z手写字体的模型。

## 连接板端
ssh 用户名@板端IP
```
# 以泰山派为例
ssh linaro@192.168.2.100
```

## 准备工作

1. 需要在板端安装编译的工具
```
sudo apt-get install -y gcc g++ cmake git make
```
2. 克隆代码
```
git clone 项目地址
```

## 编译

```
cd project
mkdir build && cd build
cmake ..
make -j 4
```

## 运行
```
./rknn_mobilenet_demo_Linux
```


