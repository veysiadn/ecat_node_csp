/******************************************************************************
 *
 *  $Id$
 *
 *  Copyright (C) 2021 Veysi ADIN, UST KIST
 *
 *  This file is part of the IgH EtherCAT master userspace program in the ROS2 environment.
 *
 *  The IgH EtherCAT master userspace program in the ROS2 environment is free software; you can
 *  redistribute it and/or modify it under the terms of the GNU General
 *  Public License as published by the Free Software Foundation; version 2
 *  of the License.
 *
 *  The IgH EtherCAT master userspace program in the ROS2 environment is distributed in the hope that
 *  it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 *  warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with the IgH EtherCAT master userspace program in the ROS environment. If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 *  ---
 *
 *  The license mentioned above concerns the source code only. Using the
 *  EtherCAT technology and brand is only permitted in compliance with the
 *  industrial property and similar rights of Beckhoff Automation GmbH.
 *
 *  Contact information: veysi.adin@kist.re.kr
 *****************************************************************************/
/*****************************************************************************
 * \file  main_window.hpp
 * \brief MainWindow implementation to show slave status and controller commands in ROS2.
 *        All GUI functionality and updating GUI handled by this MainWindow class.
 *******************************************************************************/
#pragma once

//CPP
#include <chrono>
#include <memory>
#include <iostream>

//ROS2
#include "rclcpp/rclcpp.hpp"

//GUI_Node Headers
#include "gui_node.hpp"
#include "video_capture.hpp"

// QT
#include "ui_main_window.h"
#include <QMainWindow>
#include <QApplication>
#include <QStandardItemModel>
#include <QTableView>
#include <QHeaderView>
#include <QString>
#include <QTextStream>
#include <QTimer>

using namespace GUI;
namespace Ui {
class MainWindow;
}
  class MainWindow : public QMainWindow {
  Q_OBJECT
  public:
    MainWindow(int argc, char** argv, QWidget *parent = nullptr);
    ~MainWindow();

  private slots:
  /**
   * @brief Resets all received values to 0.
   * 
   */
    void on_button_reset_clicked();
    /**
     * @brief Updates GUI based on timer callback in this case 25ms.
     * 
     */
    void UpdateGUI();
    /**
     * @brief Sets GUI appearance for disabled emergency button.
     */
    void setDisabledStyleSheet();
    /**
     * @brief Sets GUI appearance for enabled emergency button.
     */
    void setEnabledStyleSheet();
    /**
     * @brief Stops motor movement while maintining EtherCAT communication in case of emergency.
     */
    void on_button_emergency_clicked();

  private:
    /**
     * @brief To use ROS2 spinining functionality in our specific thread.
     *
     */
    void rosSpinThread();

    Ui::MainWindow *ui;
    int argc_;
    char** argv_;
    /**
     * @brief timer to update GUI in specific intervals.In this case 25ms.
     * 
     */
    QTimer my_timer;
    // To get data from gui_node_ .
    std::shared_ptr<GuiNode> gui_node_;
    VideoCapture* opencv_video_cap;
    // Thread for ROS2 spinning.
    std::thread ros_spin_thread_;
  };
