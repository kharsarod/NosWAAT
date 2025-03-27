/*
Navicat MySQL Data Transfer

Source Server         : NosTale
Source Server Version : 50527
Source Host           : localhost:3306
Source Database       : nosxplode_01

Target Server Type    : MYSQL
Target Server Version : 50527
File Encoding         : 65001

Date: 2013-01-28 21:00:01
*/

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for `tbl_accounts`
-- ----------------------------
DROP TABLE IF EXISTS `tbl_accounts`;
CREATE TABLE `tbl_accounts` (
  `AccountID` int(11) NOT NULL AUTO_INCREMENT,
  `User` varchar(20) NOT NULL,
  `Password` varchar(20) NOT NULL,
  `IsBlocked` tinyint(4) NOT NULL DEFAULT '0',
  `IsOnline` tinyint(4) NOT NULL DEFAULT '0',
  `LastSession` int(11) NOT NULL DEFAULT '0',
  `LastLoginDate` varchar(30) NOT NULL DEFAULT '-',
  `LastLoginIP` varchar(30) NOT NULL DEFAULT '-',
  PRIMARY KEY (`AccountID`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of tbl_accounts
-- ----------------------------
INSERT INTO `tbl_accounts` VALUES ('1', 'Trollface-', 'affe2', '0', '0', '30436', 'Mon Jan 28 20:51:05 2013\n', '127.0.0.1');

-- ----------------------------
-- Table structure for `tbl_characters`
-- ----------------------------
DROP TABLE IF EXISTS `tbl_characters`;
CREATE TABLE `tbl_characters` (
  `CharacterID` int(11) NOT NULL AUTO_INCREMENT,
  `AccountID` int(11) DEFAULT NULL,
  `Slot` tinyint(4) DEFAULT NULL,
  `Level` tinyint(4) DEFAULT '1',
  `JobLevel` tinyint(4) DEFAULT '1',
  `LevelEXP` int(20) DEFAULT '0',
  `JobLevelEXP` int(20) DEFAULT '0',
  `Name` varchar(20) DEFAULT NULL,
  `Gender` tinyint(4) DEFAULT NULL,
  `Hairstyle` tinyint(4) DEFAULT NULL,
  `Haircolor` tinyint(4) DEFAULT NULL,
  `Class` tinyint(4) DEFAULT '0',
  `Map` int(11) DEFAULT '1',
  `X` tinyint(4) DEFAULT NULL,
  `Y` tinyint(4) DEFAULT NULL,
  `Speed` tinyint(4) DEFAULT '11',
  `CurrentHP` mediumint(9) DEFAULT '221',
  `CurrentMP` mediumint(9) DEFAULT '60',
  `MaxHP` mediumint(9) DEFAULT '221',
  `MaxMP` mediumint(9) DEFAULT '60',
  `BackpackActive` tinyint(4) DEFAULT '0',
  `Gold` bigint(20) DEFAULT '0',
  `Inventory 0` text,
  `Inventory 1` text,
  `Inventory 2` text,
  `Friends` text,
  PRIMARY KEY (`CharacterID`),
  KEY `Account Key` (`AccountID`),
  CONSTRAINT `Account Key` FOREIGN KEY (`AccountID`) REFERENCES `tbl_accounts` (`AccountID`) ON DELETE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=8 DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of tbl_characters
-- ----------------------------
INSERT INTO `tbl_characters` VALUES ('6', '1', '1', '1', '1', '0', '0', 'Trollface-', '1', '0', '9', '0', '1', '80', '111', '11', '221', '60', '221', '60', '0', '100000', '', '', '', '7.Disconnect.0');
INSERT INTO `tbl_characters` VALUES ('7', '1', '2', '1', '1', '0', '0', 'Disconnect', '1', '1', '2', '0', '1', '81', '119', '11', '221', '60', '221', '60', '0', null, null, null, null, '6.Trollface-.0');
