---
title: PyTorch AMP Mechansim
date: 2023-11-13 10:39:33
categories: PyTorch
tags: AMP
keywords: (AMP)Automatic Mixed Precision
---

> Automatic mixed precision(AMP) 自动混合精度训练与推理，自动决定哪些算子以float16运行，哪些以float32运行，从而在降低内存以及带宽，提高计算性能的同时，减少精度损失。

## 背景

### 硬件

Nvidia 在 Volta 架构中首次引入 Tensor Core 单元，来支持 FP32 和 FP16 混合精度计算

### 模型

## 前世今生

### Nvidia apex项目

### PyTorch 1.6 内部集成 amp

## 设计

Overview 图

### 用法

### 问题

#### 如何在用户不感知的情况下选择目标算子以及进行精度转换

#### 如何解决float16的精度问题

## 实现

### 基于 Dispatcher 的注册以及分发

### 梯度缩放

## QA

### AMP 是否依赖 Tensor Core

### AMP 复制两份参数，为什么还能降低存储
