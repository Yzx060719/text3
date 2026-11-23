/* ============================================================
   医院诊疗系统用户信息管理 —— 数据库初始化脚本
   DBMS : Microsoft SQL Server 2022 (16.0) Express
   说明 : 采用第三范式设计，患者表通过外键关联科室与医生
   ============================================================ */
IF DB_ID(N'HospitalDB') IS NOT NULL
BEGIN
    ALTER DATABASE HospitalDB SET SINGLE_USER WITH ROLLBACK IMMEDIATE;
    DROP DATABASE HospitalDB;
END
GO
CREATE DATABASE HospitalDB;
GO
USE HospitalDB;
GO

/* 1. 科室表 */
CREATE TABLE dbo.Department
(
    DeptID    INT IDENTITY(1,1) NOT NULL,
    DeptName  NVARCHAR(30)      NOT NULL,
    Location  NVARCHAR(30)      NULL,
    Phone     VARCHAR(20)       NULL,
    CONSTRAINT PK_Department PRIMARY KEY (DeptID),
    CONSTRAINT UQ_Department_Name UNIQUE (DeptName)
);
GO

/* 2. 医生表（隶属科室，一对多） */
CREATE TABLE dbo.Doctor
(
    DoctorID    INT IDENTITY(1,1) NOT NULL,
    EmployeeNo  VARCHAR(10)       NOT NULL,
    DoctorName  NVARCHAR(20)      NOT NULL,
    DeptID      INT               NOT NULL,
    Title       NVARCHAR(20)      NULL,
    Phone       VARCHAR(20)       NULL,
    Specialty   NVARCHAR(50)      NULL,
    CONSTRAINT PK_Doctor PRIMARY KEY (DoctorID),
    CONSTRAINT UQ_Doctor_EmpNo UNIQUE (EmployeeNo),
    CONSTRAINT FK_Doctor_Department FOREIGN KEY (DeptID) REFERENCES dbo.Department(DeptID)
);
GO

/* 3. 患者表（核心业务表） */
CREATE TABLE dbo.Patient
(
    PatientID    INT IDENTITY(1,1) NOT NULL,
    IdCard       VARCHAR(18)       NOT NULL,
    PatientName  NVARCHAR(20)      NOT NULL,
    Sex          TINYINT           NOT NULL CONSTRAINT DF_Patient_Sex DEFAULT (1), -- 1男 0女
    DOB          DATE              NULL,
    Age          INT               NULL,
    Height       DECIMAL(5,1)      NULL,
    Weight       DECIMAL(5,1)      NULL,
    MobilePhone  VARCHAR(20)       NULL,
    Address      NVARCHAR(60)      NULL,
    DeptID       INT               NULL,
    DoctorID     INT               NULL,
    Diagnosis    NVARCHAR(120)     NULL,
    VisitDate    DATE              NULL,
    CreatedAt    DATETIME2(0)      NOT NULL CONSTRAINT DF_Patient_Created DEFAULT (SYSDATETIME()),
    CONSTRAINT PK_Patient PRIMARY KEY (PatientID),
    CONSTRAINT UQ_Patient_IdCard UNIQUE (IdCard),
    CONSTRAINT CK_Patient_Sex CHECK (Sex IN (0,1)),
    CONSTRAINT CK_Patient_Age CHECK (Age IS NULL OR (Age >= 0 AND Age <= 150)),
    CONSTRAINT FK_Patient_Department FOREIGN KEY (DeptID) REFERENCES dbo.Department(DeptID),
    CONSTRAINT FK_Patient_Doctor FOREIGN KEY (DoctorID) REFERENCES dbo.Doctor(DoctorID)
);
GO

/* 4. 系统用户表（登录用，口令存 SHA-256 摘要） */
CREATE TABLE dbo.Users
(
    UserID    INT IDENTITY(1,1) NOT NULL,
    UserName  NVARCHAR(20)      NOT NULL,
    Password  VARCHAR(64)       NOT NULL,   -- SHA-256 十六进制摘要
    FullName  NVARCHAR(20)      NULL,
    Role      NVARCHAR(10)      NULL,
    CreatedAt DATETIME2(0)      NOT NULL CONSTRAINT DF_Users_Created DEFAULT (SYSDATETIME()),
    CONSTRAINT PK_Users PRIMARY KEY (UserID),
    CONSTRAINT UQ_Users_Name UNIQUE (UserName)
);
GO

/* 5. 操作日志表（选做：记录增删改事件） */
CREATE TABLE dbo.OperationHistory
(
    HistoryID BIGINT IDENTITY(1,1) NOT NULL,
    UserName  NVARCHAR(20)         NOT NULL,
    Event     NVARCHAR(100)        NOT NULL,
    Detail    NVARCHAR(120)        NULL,
    LogTime   DATETIME2(0)         NOT NULL CONSTRAINT DF_History_Time DEFAULT (SYSDATETIME()),
    CONSTRAINT PK_OperationHistory PRIMARY KEY (HistoryID)
);
GO

/* ===================== 初始数据 ===================== */
INSERT INTO dbo.Department (DeptName, Location, Phone) VALUES
 (N'内科',   N'门诊楼2层', '0769-22861001'),
 (N'外科',   N'门诊楼3层', '0769-22861002'),
 (N'儿科',   N'门诊楼1层', '0769-22861003'),
 (N'骨科',   N'住院楼5层', '0769-22861004'),
 (N'急诊科', N'急诊楼1层', '0769-22861005');
GO

INSERT INTO dbo.Doctor (EmployeeNo, DoctorName, DeptID, Title, Phone, Specialty) VALUES
 ('D001', N'陈志远', 1, N'主任医师', '13800138001', N'心血管内科'),
 ('D002', N'林晓萍', 1, N'副主任医师','13800138002', N'呼吸内科'),
 ('D003', N'黄建国', 2, N'主任医师', '13800138003', N'普外科'),
 ('D004', N'吴敏',   3, N'主治医师', '13800138004', N'小儿呼吸道疾病'),
 ('D005', N'周立群', 4, N'副主任医师','13800138005', N'关节与脊柱'),
 ('D006', N'郑海涛', 5, N'主治医师', '13800138006', N'急诊创伤');
GO

INSERT INTO dbo.Patient (IdCard, PatientName, Sex, DOB, Age, Height, Weight, MobilePhone, Address, DeptID, DoctorID, Diagnosis, VisitDate) VALUES
 ('441900199003120011', N'张伟',   1, '1990-03-12', 36, 175.0, 70.5, '13900001001', N'东莞市莞城街道',   1, 1, N'高血压2级',        '2026-11-02'),
 ('441900198507250022', N'李秀兰', 0, '1985-07-25', 41, 160.0, 55.0, '13900001002', N'东莞市南城街道',   1, 2, N'社区获得性肺炎',  '2026-11-05'),
 ('441900201203180033', N'王小虎', 1, '2012-03-18', 14, 150.0, 42.0, '13900001003', N'东莞市东城街道',   3, 4, N'急性支气管炎',    '2026-11-08'),
 ('441900197811090044', N'刘建国', 1, '1978-11-09', 48, 172.0, 78.0, '13900001004', N'东莞市万江街道',   2, 3, N'急性阑尾炎术后',  '2026-11-10'),
 ('441900199506140055', N'陈静怡', 0, '1995-06-14', 31, 165.0, 52.0, '13900001005', N'东莞市寮步镇',     1, 2, N'过敏性哮喘',      '2026-11-12'),
 ('441900196502220066', N'赵国强', 1, '1965-02-22', 61, 170.0, 75.0, '13900001006', N'东莞市石龙镇',     4, 5, N'腰椎间盘突出',    '2026-11-14'),
 ('441900200010300077', N'孙雅婷', 0, '2000-10-30', 26, 168.0, 50.0, '13900001007', N'东莞市长安镇',     5, 6, N'踝关节扭伤',      '2026-11-15'),
 ('441900198304170088', N'周文博', 1, '1983-04-17', 43, 178.0, 82.0, '13900001008', N'东莞市虎门镇',     2, 3, N'腹股沟疝',        '2026-11-18'),
 ('441900199209080099', N'吴丽娟', 0, '1992-09-08', 34, 162.0, 58.0, '13900001009', N'东莞市厚街镇',     3, 4, N'小儿肺炎随访',    '2026-11-19'),
 ('441900197607120100', N'郑建华', 1, '1976-07-12', 50, 171.0, 73.0, '13900001010', N'东莞市常平镇',     1, 1, N'冠状动脉粥样硬化', '2026-11-20');
GO

/* 初始账号：admin / 123456 ；doctor01 / 888888 （口令以 SHA-256 十六进制保存） */
INSERT INTO dbo.Users (UserName, Password, FullName, Role) VALUES
 ('admin',    CONVERT(VARCHAR(64), HASHBYTES('SHA2_256', '123456'), 2), N'系统管理员', N'管理员'),
 ('doctor01', CONVERT(VARCHAR(64), HASHBYTES('SHA2_256', '888888'), 2), N'陈志远',     N'医生');
GO

INSERT INTO dbo.OperationHistory (UserName, Event, Detail) VALUES
 (N'admin', N'系统初始化', N'创建数据库与初始数据');
GO

/* 校验：查看表清单与行数 */
SELECT t.name AS 表名, SUM(p.rows) AS 行数
FROM sys.tables t JOIN sys.partitions p ON p.object_id = t.object_id AND p.index_id IN (0,1)
GROUP BY t.name ORDER BY t.name;
GO
