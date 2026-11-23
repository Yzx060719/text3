# HospitalMS —— 基于 Qt SQL 与 View/Model 的医院诊疗信息管理系统

东莞理工学院 《Qt 应用程序开发》 实验三：QtSQL 数据库模块与 View-Model 结构的使用。

## 功能概览

| 模块 | 说明 | 对应要求 |
|---|---|---|
| 登录控制 | `Users` 表校验，SHA-256 口令摘要，区分「用户不存在 / 密码错误 / 数据库不可用」 | 必做 |
| 欢迎首页 | 三张导航卡片 + 日期问候 + 操作日志入口 | 必做（界面切换） |
| 患者管理 | `QSqlTableModel` 列表、条件查询、新增、修改、删除、CSV 导出 | 必做 |
| 科室 / 医生管理 | 可编辑表格 + 外键下拉代理，增删行后批量提交 | 选做 |
| 操作日志 | 登录与增删改操作写入 `OperationHistory`，支持按用户/事件检索 | 选做 |
| 统计查询 | 科室患者分布、医生接诊量、本月新增等聚合统计 | 扩展 |

## 目录结构

```
HospitalMS/
├─ HospitalMS.pro          # qmake 工程文件（QT += widgets sql）
├─ resources.qrc           # 样式表资源
├─ sql/create_db.sql       # 建库、建表、约束与初始数据脚本
├─ src/
│  ├─ main.cpp             # 入口：先连库再建窗口，加载样式与中文翻译
│  ├─ dbmanager.h/.cpp     # 数据库单例（QODBC）、日志写入、SHA-256
│  ├─ delegates.h/.cpp     # 性别 / 科室 / 医生 / 日期 自定义代理
│  ├─ loginpage.h/.cpp     # 登录页
│  ├─ homepage.h/.cpp      # 欢迎首页与导航卡片
│  ├─ patientpage.h/.cpp   # 患者列表、查询、删除、导出
│  ├─ patientdialog.h/.cpp # 患者新增 / 修改对话框
│  ├─ managepage.h/.cpp    # 科室 / 医生管理
│  ├─ statisticsdialog.*   # 统计查询
│  ├─ logpage.h/.cpp       # 操作日志
│  ├─ mainwindow.h/.cpp    # QStackedWidget 主框架与工具栏
│  └─ style.qss            # 全局浅色卡片式样式
└─ shots/                  # 实验报告截图
```

## 编译与运行

1. 用 SSMS 执行 `sql/create_db.sql` 创建 `HospitalDB`。
2. Qt Creator 打开 `HospitalMS.pro`（Qt 6.x MinGW/MSVC 64 位），构建运行。
3. 默认连接串：`DRIVER={ODBC Driver 17 for SQL Server};SERVER=localhost;DATABASE=HospitalDB;Trusted_Connection=yes;`
   可用环境变量 `HOSPITALMS_SERVER` / `HOSPITALMS_DB`，或命令行 `--db=<库名>` 覆盖，用于演示连接失败分支。

## 演示账号

| 用户名 | 口令 | 角色 |
|---|---|---|
| admin | 123456 | 管理员 |
| doctor01 | 888888 | 医生 |

## 快捷键

`Ctrl+F` 查询 · `Ctrl+N` 新增 · `Ctrl+E` 修改 · `Ctrl+D` 删除 · `Ctrl+T` 统计 · `Ctrl+Shift+E` 导出 · `F5` 刷新 · `Ctrl+1~5` 页面切换
