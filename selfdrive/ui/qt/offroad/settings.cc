#include "selfdrive/ui/qt/offroad/settings.h"

#include <cassert>
#include <string>

#include <QDebug>

#ifndef QCOM
#include "selfdrive/ui/qt/offroad/networking.h"
#endif

#ifdef ENABLE_MAPS
#include "selfdrive/ui/qt/maps/map_settings.h"
#endif

#include "selfdrive/common/params.h"
#include "selfdrive/common/util.h"
#include "selfdrive/hardware/hw.h"
#include "selfdrive/ui/qt/widgets/controls.h"
#include "selfdrive/ui/qt/widgets/input.h"
#include "selfdrive/ui/qt/widgets/scrollview.h"
#include "selfdrive/ui/qt/widgets/ssh_keys.h"
#include "selfdrive/ui/qt/widgets/toggle.h"
#include "selfdrive/ui/ui.h"
#include "selfdrive/ui/qt/util.h"
#include "selfdrive/ui/qt/qt_window.h"

TogglesPanel::TogglesPanel(QWidget *parent) : QWidget(parent) {
  QVBoxLayout *main_layout = new QVBoxLayout(this);

  QList<ParamControl*> toggles;

  toggles.append(new ParamControl("OpenpilotEnabledToggle",
                                  "Enable openpilot",
                                  "Use the openpilot system for adaptive cruise control and lane keep driver assistance. Your attention is required at all times to use this feature. Changing this setting takes effect when the car is powered off.",
                                  "../assets/offroad/icon_openpilot.png",
                                  this));

  toggles.append(new ParamControl("DisableUpdates",
                                  "Disable automatic updates",
                                  "When this is enabled, openpilot will not check for or install updates.",
                                  "../assets/offroad/icon_openpilot.png",
                                  this));

  toggles.append(new ParamControl("MetricResetSwitch",
                                  "On-screen information (tap me; reset)",
                                  "When the car is on, tap the current speed to unlock edit mode, then tap more to cycle the number of metrics shown on the right side. Tap each metric to change the information displayed. Use this toggle to, upon the next vehicle start, reset the distance travelled, disengagement/intervention/interaction/distraction counts, and EV consumption and efficiency trip and 5mi/8km metrics.",
                                  "../assets/offroad/icon_calibration.png",
                                  this));
  toggles.append(new ParamControl("DisableDisengageOnGas",
                                  "Steer when gas pressed",
                                  "Keep steering when gas (accelerator) is pressed",
                                  "../assets/offroad/icon_car_pedal.png",
                                  this));
  toggles.append(new ParamControl("MADSEnabled",
                                  "[GM] MADS (tap me)",
                                  "MADS = Modified Assistive Driving Safety. Three independently toggle-able, always-on features: Autosteer, Lead braking, and One-pedal mode. This toggle must be enabled to use any of them. Toggle autosteer (even when brakes are pressed and before initial engagement) using the lane keep assist steering wheel button. Use cruise main to shut off all OP control. When MADS is running, the max speed is replaced by the MADS icon",
                                  "../assets/offroad/icon_car_MADS.png",
                                  this));
  toggles.append(new ParamControl("MADSLeadBraking",
                                   "MADS Lead braking (tap me)",
                                   "Toggle by pressing the ACC distance button when MADS icon is showing. OpenPilot will smoothly stop for lead cars. Even the slightest touch of gas/brake will override lead braking. When this is enabled, an additional white circle will appear around the MADS icon.",
                                   "../assets/offroad/icon_car_MADS.png",
                                   this));
  toggles.append(new ParamControl("MADSOnePedalMode",
                                  "MADS One-pedal mode (tap me)",
                                  "Toggle by double-pressing the Volt regen paddle or tapping the MADS icon. When active, the MADS icon will change to the colored one-pedal icon, and op will apply light braking to bring you to a stop when you're not pressing the gas pedal in L mode. One-pedal one-time stop: Hold the Volt regen paddle to below 5mph and one pedal mode will engage temporarily to stop you once, then turn off when you resume driving.",
                                  "../assets/offroad/icon_car_pedal.png",
                                  this));
  toggles.append(new ParamControl("MADSPauseBlinkerSteering",
                                  "MADS no slow blinker steer",
                                  "When MADS is active, under 20mph with the blinker on and decelerating, steering pauses to make it easier to perform sharp turns (which may be contrary to OpenPilot's plan). Steering is unpaused when you resume acceleration or go above 20mph again.",
                                  "../assets/offroad/icon_hands_on_wheel.png",
                                  this));

  toggles.append(new ParamControl("OPParamsLiveTuneEnabled",
                                  "Customization over SSH (tap me)",
                                  "opParams: Many of the features can be fine-tuned to your liking by connecting to your device over SSH and running `./opparams.py` from the `/data/openpilot/` directory. Most of the adjustable parameters are live-tunable—can be modified while driving and take effect immediately IF THIS TOGGLE IS ENABLED. If this toggle is NOT enabled, then all parameters become \"startup\" parameters where a car or OpenPilot restart is necessary for them to take effect. Changes to this toggle will take effect the next time you start your car, and you'll need to restart `opparams.py` for it to reflect the change.",
                                  "../assets/offroad/icon_calibration.png",
                                  this));

  toggles.append(new ParamControl("OPParamsLateralOverride",
                                  "Custom lateral override (tap me)",
                                  "This enables the lateral tune specified in opParams.",
                                  "../assets/offroad/icon_calibration.png",
                                  this));

  toggles.append(new ParamControl("OPParamsLongitudinalOverride",
                                  "Custom long override (tap me)",
                                  "This enables the longitudinal tune specified in opParams.",
                                  "../assets/offroad/icon_calibration.png",
                                  this));

  toggles.append(new ParamControl("OPParamsReset",
                                  "Reset OPParams to defaults",
                                  "If enabled, then the next time OPParams runs, by starting openpilot or by running ./opparams.py, all parameters will be reset to their default values. This toggle will then be disabled.",
                                  "../assets/offroad/icon_calibration.png",
                                  this));
  
  toggles.append(new ParamControl("LongRangeLeadsEnabled",
                                  "Longer-range lead detection (beta)",
                                  "This also allows for 10-20\% longer range detection using radar and the cars LKA camera.",
                                  "../assets/offroad/icon_plus.png",
                                  this));

  toggles.append(new ParamControl("ExtendedRadar",
                                  "Extended radar capabilities (beta)",
                                  "Enables tracking of all cars; not just the one openpilot lead. Necessary for braking for car in front of lead, longer-range lead detection, traffic-based auto lane position, drawing of oncoming/ongoing lanes, and indication of non-lead cars.",
                                  "../assets/offroad/icon_plus.png",
                                  this));
  toggles.append(new ParamControl("TurnVisionControl",
                                  "Enable vision based turn control",
                                  "Use vision path predictions to estimate the appropiate speed to drive through turns ahead.",
                                  "../assets/offroad/icon_slow_curves_vision.png",
                                  this));
  toggles.append(new ParamControl("TurnSpeedControl ⚠️ Tap me! ⚠️",
                                  "Enable Map Data Turn Control",
                                  "ONLY USE THIS FEATURE IF YOU HAVE A DATA HOTSPOT OR YOUR OWN SIM CARD. DO NOT USE WITH COMMA PRIME OR COMMA WILL TERMINATE YOUR COMMA PRIME SUBSCRIPTION! Use curvature info from map data to define speed limits to take turns ahead",
                                  "../assets/offroad/icon_slow_curves_map.png",
                                  this));
  toggles.append(new ParamControl("SpeedLimitControl ⚠️ Tap me! ⚠️",
                                  "Enable Speed Limit Control",
                                  "ONLY USE THIS FEATURE IF YOU HAVE A DATA HOTSPOT OR YOUR OWN SIM CARD. DO NOT USE WITH COMMA PRIME OR COMMA WILL TERMINATE YOUR COMMA PRIME SUBSCRIPTION! Use speed limit signs information from map data and car interface to automatically adapt cruise speed to road limits.",
                                  "../assets/offroad/icon_speed_limit_sign.png",
                                  this));
  toggles.append(new ParamControl("EUSpeedLimitStyle",
                                  "Show EU style speed limit sign",
                                  "If enabled, show EU style circular sign. If disabled, show US/Canada style rectangular sign.",
                                  "../assets/offroad/icon_speed_limit_sign.png",
                                  this));
  toggles.append(new ParamControl("SpeedLimitPercOffset",
                                  "Enable Speed Limit Offset",
                                  "Set speed limit slightly higher than actual speed limit for a more natural drive.",
                                  "../assets/offroad/icon_speed_limit_percent.png",
                                  this));
  toggles.append(new ParamControl("ReverseSpeedAdjust",
                                  "Reverse cruise speed adjustment",
                                  "Reverse of stock behavior, press/hold the accel/decel buttons to change by 5mph/1mph.",
                                  "../assets/offroad/icon_stock_adjust_speed.png",
                                  this));
  toggles.append(new ParamControl("CruiseSpeedOffset",
                                  "Enable Cruise Speed Offset (+3mph)",
                                  "When adjusting, cruise speed will be {8, 13, 18, 23, 28} mph.",
                                  "../assets/offroad/icon_speed_offset.png",
                                  this));
  toggles.append(new ParamControl("LanePositionEnabled",
                                  "Adjustable lane position",
                                  "Adds onscreen arrows to the left and right sides of the onroad screen that can be used to adjust lane position temporarily. Tap both arrows in succession to enable automatic mode that keeps you away from other traffic when in the far-left or far-right lanes.",
                                  "../assets/offroad/icon_road.png",
                                  this));
  toggles.append(new ParamControl("AutoAutoLanePosition",
                                  "Auto auto lane position",
                                  "Automatically enable automatic lane position when you're going 10mph+, lanelines are clear, and there's traffic. Automatic lane position keeps you farther from adjacent traffic when possible/safe.",
                                  "../assets/offroad/icon_road.png",
                                  this));
  toggles.append(new ParamControl("AccelModeButton",
                                  "Acceleration profiles",
                                  "Cycle between normal, sport, and eco acceleration profiles.",
                                  "../assets/offroad/icon_rocket.png",
                                  this));
  toggles.append(new ParamControl("DynamicFollowToggle",
                                  "Dynamic follow",
                                  "Automatically (and imperceptibly) switch between close/medium/far follow profiles based on speed and traffic.",
                                  "../assets/offroad/icon_road.png",
                                  this));
  toggles.append(new ParamControl("NudgelessLaneChange",
                                  "Nudgeless lane change (1s delay) ⚠️",
                                  "Perform lane change without requiring nudge from driver",
                                  "../assets/offroad/icon_hands_on_wheel.png",
                                  this));
  toggles.append(new ParamControl("GMAutoHold",
                                  "[GM] Enable AutoHold",
                                  "Holds brakes automatically after coming to a complete stop, even when OP is disengaged.",
                                  "../assets/offroad/icon_gm_autohold.png",
                                  this));
  toggles.append(new ParamControl("Coasting",
                                  "[GM] Coasting (tap me)",
                                  "OP will allow the car to coast above the set speed rather than use engine/regen/friction braking. If this toggle is enabled, then you can toggle coasting while driving by tapping the on-screen max speed indicator, but you can only disable coasting while driving if you're traveling below your set speed (or at any speed if you have the \"Engine/regen braking\" toggle enabled).",
                                  "../assets/offroad/icon_car_pedal.png",
                                  this));
  toggles.append(new ParamControl("CoastingBrakeOverSpeed",
                                  "[GM] Coast: brake 15% over set speed",
                                  "When coasting, start applying cruise braking when 15% over set speed.",
                                  "../assets/offroad/icon_speed_offset.png",
                                  this));
  toggles.append(new ParamControl("CoastingDL",
                                  "[Volt] D/L coast control",
                                  "Tie the above option to the D/L gear shifter position. Coast in D; maintain set speed exactly in L.",
                                  "../assets/offroad/icon_gear_shifter.png",
                                  this));
  toggles.append(new ParamControl("RegenBraking",
                                  "[GM] Engine/regen braking",
                                  "Disable friction braking when OP is slowing to maintain cruise/speed limit; still brake for following/curves",
                                  "../assets/img_brake.png",
                                  this));
  toggles.append(new ParamControl("BrakeIndicator",
                                  "[GM] Power/Brake indicator",
                                  "Brake indicator at bottom-right when driving or power meter to right. Tap indicator or meter to change. Circle at indicator center grows and turns red to indicate the level of braking. Pulses immediately after starting car to let you know it's on.",
                                  "../assets/offroad/icon_brake_disc.png",
                                  this));
  toggles.append(new ParamControl("CustomSounds",
                                  "Alternative sounds",
                                  "Uses alternative set of sound effects.",
                                  "../assets/offroad/icon_custom_sounds.png",
                                  this));
  toggles.append(new ParamControl("SilentEngageDisengage",
                                  "Silent engage/disengage",
                                  "Mute engage and disengage sounds.",
                                  "../assets/offroad/icon_mute.png",
                                  this));
  toggles.append(new ParamControl("IgnoreMissingNVME",
                                  "Ignore missing NVME",
                                  "Prevent an error about missing NVME drive from showing on 32GB C3's. (restart device for change to take effect)",
                                  "../assets/offroad/icon_settings.png",
                                  this));
  toggles.append(new ParamControl("GrayPandaSupport",
                                  "Gray panda support",
                                  "Necessary to run on gray panda",
                                  "../assets/offroad/icon_settings.png",
                                  this));
  toggles.append(new ParamControl("FPVolt",
                                  "Volt fingerprint",
                                  "Forces Volt fingerprint",
                                  "../assets/offroad/icon_settings.png",
                                  this));
  toggles.append(new ParamControl("LowOverheadMode",
                                  "Lower device overhead",
                                  "Decreases device power, CPU, and storage use for running better on older hardware by: 1) defaulting to medium brightness (tap DM icon to change), 2) disable onroad logging (loggerd and proclogd). Your device will not keep or upload logs with this enabled!",
                                  "../assets/offroad/icon_settings.png",
                                  this));
  toggles.append(new ParamControl("WeatherDisplayEnabled",
                                  "Display weather info",
                                  "Tap weather info to cycle between simple and full layouts. If you want, enter your own OpenWeatherMap.org api key into /data/OpenWeatherMap_apiKey.txt",
                                  "../assets/weather/10n.png",
                                  this));
  toggles.append(new ParamControl("WeatherSafetyEnabled",
                                  "Weather safety",
                                  "OpenPilot will automatically use softer acceleration, softer curve braking, follow traffic farther, or lower the speed limit offset in use. If you want, enter your own OpenWeatherMap.org api key using opparams (connect over ssh and run `./opparams`)",
                                  "../assets/weather/10n.png",
                                  this));
  toggles.append(new ParamControl("AutoBrightness",
                                  "Auto brightness",
                                  "Set brightness automatically. High during the day and medium at night, after sunset. You can override this until the next car start by manually changing brightness by tapping the face icon at bottom-left.",
                                  "../assets/offroad/icon_metric.png",
                                  this));
  toggles.append(new ParamControl("ColorPath",
                                  "Colored path",
                                  "Color path according to the amount of lateral (steering) correction being applied",
                                  "../assets/offroad/icon_road.png",
                                  this));
  toggles.append(new ParamControl("AlternateColors",
                                  "Alternate colors",
                                  "Use alternate color set.",
                                  "../assets/offroad/icon_road.png",
                                  this));
  toggles.append(new ParamControl("AdjacentPaths",
                                  "Adjacent oncoming/ongoing paths",
                                  "[Requires extended radar toggle] Draw paths to indicate whether adjacent lanes contain oncoming (red) or ongoing (green) traffic.",
                                  "../assets/offroad/icon_road.png",
                                  this));
  toggles.append(new ParamControl("PrintCurrentSpeed",
                                  "Print current speed",
                                  "Print current vehicle speed on Comma device screen",
                                  "../assets/offroad/icon_metric.png",
                                  this));
  toggles.append(new ParamControl("PrintLeadInfo",
                                  "Print lead car info",
                                  "Prints lead car time and length distance, and absolute and relative velocity next to lead indicator",
                                  "../assets/offroad/icon_metric.png",
                                  this));
  toggles.append(new ParamControl("PrintAdjacentLeadSpeeds",
                                  "Indicate all cars",
                                  "[Requires extended radar toggle] Print speeds of all cars being tracked by radar and/or vision. Tap at screen bottom in the middle of the path to toggle display modes between printing inside the indicator or along the bottom of the screen, out from the center to the left/right in order of distance, so the numbers closest to the center are for the more immediate cars. Cars are also indicated onscreen as oncoming (red) or ongoing (green).",
                                  "../assets/offroad/icon_metric.png",
                                  this));

  toggles.append(new ParamControl("EnableTorqueControl",
                                  "Enable \"torque\" steering control",
                                  "(Restart car to take effect) Use the newer torque-based steering control that steers by achieving a target amount of lateral acceleration rather than achieving a target steering angle. Torque tune is only available in the Volt.",
                                  "../assets/offroad/icon_openpilot.png",
                                  this));

  toggles.append(new ParamControl("EnableNNFF",
                                  "Neural network feedforward",
                                  "(Restart car to take effect) Use the experimental neural network feedforward for both torque or lat_pid lateral control.",
                                  "../assets/offroad/icon_openpilot.png",
                                  this));


  toggles.append(new ParamControl("IsLdwEnabled",
                                  "Enable Lane Departure Warnings",
                                  "Receive alerts to steer back into the lane when your vehicle drifts over a detected lane line without a turn signal activated while driving over 31mph (50kph).",
                                  "../assets/offroad/icon_warning.png",
                                  this));
  toggles.append(new ParamControl("IsRHD",
                                  "Enable Right-Hand Drive",
                                  "Allow openpilot to obey left-hand traffic conventions and perform driver monitoring on right driver seat.",
                                  "../assets/offroad/icon_openpilot_mirrored.png",
                                  this));
  toggles.append(new ParamControl("IsMetric",
                                  "Use Metric System",
                                  "Display speed in km/h instead of mp/h.",
                                  "../assets/offroad/icon_metric.png",
                                  this));

  toggles.append(new ParamControl("UploadRaw",
                                  "Upload Raw Logs",
                                  "Upload full logs and full resolution video by default while on WiFi. If not enabled, individual logs can be marked for upload at my.comma.ai/useradmin.",
                                  "../assets/offroad/icon_network.png",
                                  this));

  toggles.append(new ParamControl("DisableOnroadUploads",
                                  "Disable onroad uploads",
                                  "Completely disable uploads when onroad. Necessary to avoid high data use when connected to wifi hotspot.",
                                  "../assets/offroad/icon_network.png",
                                  this));

  ParamControl *record_toggle = new ParamControl("RecordFront",
                                                 "Record and Upload Driver Camera",
                                                 "Upload data from the driver facing camera and help improve the driver monitoring algorithm.",
                                                 "../assets/offroad/icon_monitoring.png",
                                                 this);
  toggles.append(record_toggle);
  toggles.append(new ParamControl("EndToEndToggle",
                                  "\U0001f96c Disable use of lanelines (Alpha) \U0001f96c",
                                  "In this mode openpilot will ignore lanelines and just drive how it thinks a human would.",
                                  "../assets/offroad/icon_road.png",
                                  this));
  
  toggles.append(new ParamControl("HandsOnWheelMonitoring",
                                  "Enable Hands on Wheel Monitoring",
                                  "Monitor and alert when driver is not keeping the hands on the steering wheel.",
                                  "../assets/offroad/icon_hands_on_wheel.png",
                                  this));
  toggles.append(new ParamControl("ShowDebugUI",
                                  "Show debug UI elements",
                                  "Show UI elements that aid debugging.",
                                  "../assets/offroad/icon_calibration.png",
                                  this));

#ifdef ENABLE_MAPS
  toggles.append(new ParamControl("NavSettingTime24h",
                                  "Show ETA in 24h format",
                                  "Use 24h format instead of am/pm",
                                  "../assets/offroad/icon_metric.png",
                                  this));
#endif

  bool record_lock = Params().getBool("RecordFrontLock");
  record_toggle->setEnabled(!record_lock);

  for(ParamControl *toggle : toggles) {
    if(main_layout->count() != 0) {
      main_layout->addWidget(horizontal_line());
    }
    main_layout->addWidget(toggle);
  }
}

DevicePanel::DevicePanel(QWidget* parent) : QWidget(parent) {
  QVBoxLayout *main_layout = new QVBoxLayout(this);
  Params params = Params();
  main_layout->addWidget(new LabelControl("Dongle ID", getDongleId().value_or("N/A")));
  main_layout->addWidget(horizontal_line());

  QString serial = QString::fromStdString(params.get("HardwareSerial", false));
  main_layout->addWidget(new LabelControl("Serial", serial));

  // offroad-only buttons

  auto dcamBtn = new ButtonControl("Driver Camera", "PREVIEW",
                                        "Preview the driver facing camera to help optimize device mounting position for best driver monitoring experience. (vehicle must be off)");
  connect(dcamBtn, &ButtonControl::clicked, [=]() { emit showDriverView(); });

  QString resetCalibDesc = "openpilot requires the device to be mounted within 4° left or right and within 5° up or down. openpilot is continuously calibrating, resetting is rarely required.";
  auto resetCalibBtn = new ButtonControl("Reset Calibration", "RESET", resetCalibDesc);
  connect(resetCalibBtn, &ButtonControl::clicked, [=]() {
    if (ConfirmationDialog::confirm("Are you sure you want to reset calibration?", this)) {
      Params().remove("CalibrationParams");
    }
  });
  connect(resetCalibBtn, &ButtonControl::showDescription, [=]() {
    QString desc = resetCalibDesc;
    std::string calib_bytes = Params().get("CalibrationParams");
    if (!calib_bytes.empty()) {
      try {
        AlignedBuffer aligned_buf;
        capnp::FlatArrayMessageReader cmsg(aligned_buf.align(calib_bytes.data(), calib_bytes.size()));
        auto calib = cmsg.getRoot<cereal::Event>().getLiveCalibration();
        if (calib.getCalStatus() != 0) {
          double pitch = calib.getRpyCalib()[1] * (180 / M_PI);
          double yaw = calib.getRpyCalib()[2] * (180 / M_PI);
          desc += QString(" Your device is pointed %1° %2 and %3° %4.")
                                .arg(QString::number(std::abs(pitch), 'g', 1), pitch > 0 ? "up" : "down",
                                     QString::number(std::abs(yaw), 'g', 1), yaw > 0 ? "right" : "left");
        }
      } catch (kj::Exception) {
        qInfo() << "invalid CalibrationParams";
      }
    }
    resetCalibBtn->setDescription(desc);
  });

  ButtonControl *retrainingBtn = nullptr;
  if (!params.getBool("Passive")) {
    retrainingBtn = new ButtonControl("Review Training Guide", "REVIEW", "Review the rules, features, and limitations of openpilot");
    connect(retrainingBtn, &ButtonControl::clicked, [=]() {
      if (ConfirmationDialog::confirm("Are you sure you want to review the training guide?", this)) {
        Params().remove("CompletedTrainingVersion");
        emit reviewTrainingGuide();
      }
    });
  }

  ButtonControl *regulatoryBtn = nullptr;
  if (Hardware::TICI()) {
    regulatoryBtn = new ButtonControl("Regulatory", "VIEW", "");
    connect(regulatoryBtn, &ButtonControl::clicked, [=]() {
      const std::string txt = util::read_file(ASSET_PATH.toStdString() + "/offroad/fcc.html");
      RichTextDialog::alert(QString::fromStdString(txt), this);
    });
  }

  for (auto btn : {dcamBtn, resetCalibBtn, retrainingBtn, regulatoryBtn}) {
    if (btn) {
      main_layout->addWidget(horizontal_line());
      connect(parent, SIGNAL(offroadTransition(bool)), btn, SLOT(setEnabled(bool)));
      main_layout->addWidget(btn);
    }
  }

  // power buttons
  QHBoxLayout *power_layout = new QHBoxLayout();
  power_layout->setSpacing(30);

  QPushButton *reboot_btn = new QPushButton("Reboot");
  reboot_btn->setObjectName("reboot_btn");
  power_layout->addWidget(reboot_btn);
  QObject::connect(reboot_btn, &QPushButton::clicked, [=]() {
    if (ConfirmationDialog::confirm("Are you sure you want to reboot?", this)) {
      Hardware::reboot();
    }
  });

  QPushButton *poweroff_btn = new QPushButton("Power Off");
  poweroff_btn->setObjectName("poweroff_btn");
  power_layout->addWidget(poweroff_btn);
  QObject::connect(poweroff_btn, &QPushButton::clicked, [=]() {
    if (ConfirmationDialog::confirm("Are you sure you want to power off?", this)) {
      Hardware::poweroff();
    }
  });

  setStyleSheet(R"(
    QPushButton {
      height: 120px;
      border-radius: 15px;
    }
    #reboot_btn { background-color: #393939; }
    #reboot_btn:pressed { background-color: #4a4a4a; }
    #poweroff_btn { background-color: #E22C2C; }
    #poweroff_btn:pressed { background-color: #FF2424; }
  )");
  main_layout->addLayout(power_layout);
}

SoftwarePanel::SoftwarePanel(QWidget* parent) : QWidget(parent) {
  gitBranchLbl = new LabelControl("Git Branch");
  gitCommitLbl = new LabelControl("Git Commit");
  osVersionLbl = new LabelControl("OS Version");
  versionLbl = new LabelControl("Version", "", QString::fromStdString(params.get("ReleaseNotes")).trimmed());
  lastUpdateLbl = new LabelControl("Last Update Check", "", "The last time openpilot successfully checked for an update. The updater only runs while the car is off.");
  updateBtn = new ButtonControl("Check for Update", "");
  connect(updateBtn, &ButtonControl::clicked, [=]() {
    if (params.getBool("IsOffroad")) {
      fs_watch->addPath(QString::fromStdString(params.getParamPath("LastUpdateTime")));
      fs_watch->addPath(QString::fromStdString(params.getParamPath("UpdateFailedCount")));
      updateBtn->setText("CHECKING");
      updateBtn->setEnabled(false);
    }
    std::system("pkill -1 -f selfdrive.updated");
  });

  QVBoxLayout *main_layout = new QVBoxLayout(this);
  QWidget *widgets[] = {versionLbl, lastUpdateLbl, updateBtn, gitBranchLbl, gitCommitLbl, osVersionLbl};
  for (int i = 0; i < std::size(widgets); ++i) {
    main_layout->addWidget(widgets[i]);
    main_layout->addWidget(horizontal_line());
  }

  auto uninstallBtn = new ButtonControl("Uninstall " + getBrand(), "UNINSTALL");
  connect(uninstallBtn, &ButtonControl::clicked, [=]() {
    if (ConfirmationDialog::confirm("Are you sure you want to uninstall?", this)) {
      Params().putBool("DoUninstall", true);
    }
  });
  connect(parent, SIGNAL(offroadTransition(bool)), uninstallBtn, SLOT(setEnabled(bool)));
  main_layout->addWidget(uninstallBtn);

  fs_watch = new QFileSystemWatcher(this);
  QObject::connect(fs_watch, &QFileSystemWatcher::fileChanged, [=](const QString path) {
    int update_failed_count = params.get<int>("UpdateFailedCount").value_or(0);
    if (path.contains("UpdateFailedCount") && update_failed_count > 0) {
      lastUpdateLbl->setText("failed to fetch update");
      updateBtn->setText("CHECK");
      updateBtn->setEnabled(true);
    } else if (path.contains("LastUpdateTime")) {
      updateLabels();
    }
  });
}

void SoftwarePanel::showEvent(QShowEvent *event) {
  updateLabels();
}

void SoftwarePanel::updateLabels() {
  QString lastUpdate = "";
  auto tm = params.get("LastUpdateTime");
  if (!tm.empty()) {
    lastUpdate = timeAgo(QDateTime::fromString(QString::fromStdString(tm + "Z"), Qt::ISODate));
  }

  versionLbl->setText(getBrandVersion());
  lastUpdateLbl->setText(lastUpdate);
  updateBtn->setText("CHECK");
  updateBtn->setEnabled(true);
  gitBranchLbl->setText(QString::fromStdString(params.get("GitBranch")));
  gitCommitLbl->setText(QString::fromStdString(params.get("GitCommit")).left(10));
  osVersionLbl->setText(QString::fromStdString(Hardware::get_os_version()).trimmed());
}

QWidget * network_panel(QWidget * parent) {
#ifdef QCOM
  QWidget *w = new QWidget(parent);
  QVBoxLayout *layout = new QVBoxLayout(w);
  layout->setSpacing(30);

  // wifi + tethering buttons
  auto wifiBtn = new ButtonControl("WiFi Settings", "OPEN");
  QObject::connect(wifiBtn, &ButtonControl::clicked, [=]() { HardwareEon::launch_wifi(); });
  layout->addWidget(wifiBtn);
  layout->addWidget(horizontal_line());

  auto tetheringBtn = new ButtonControl("Tethering Settings", "OPEN");
  QObject::connect(tetheringBtn, &ButtonControl::clicked, [=]() { HardwareEon::launch_tethering(); });
  layout->addWidget(tetheringBtn);
  layout->addWidget(horizontal_line());

  // SSH key management
  layout->addWidget(new SshToggle());
  layout->addWidget(horizontal_line());
  layout->addWidget(new SshControl());

  layout->addStretch(1);
#else
  Networking *w = new Networking(parent);
#endif
  return w;
}

void SettingsWindow::showEvent(QShowEvent *event) {
  panel_widget->setCurrentIndex(0);
  nav_btns->buttons()[0]->setChecked(true);
}

SettingsWindow::SettingsWindow(QWidget *parent) : QFrame(parent) {

  // setup two main layouts
  sidebar_widget = new QWidget;
  QVBoxLayout *sidebar_layout = new QVBoxLayout(sidebar_widget);
  sidebar_layout->setMargin(0);
  panel_widget = new QStackedWidget();
  panel_widget->setStyleSheet(R"(
    border-radius: 30px;
    background-color: #292929;
  )");

  // close button
  QPushButton *close_btn = new QPushButton("×");
  close_btn->setStyleSheet(R"(
    QPushButton {
      font-size: 140px;
      padding-bottom: 20px;
      font-weight: bold;
      border 1px grey solid;
      border-radius: 100px;
      background-color: #292929;
      font-weight: 400;
    }
    QPushButton:pressed {
      background-color: #3B3B3B;
    }
  )");
  close_btn->setFixedSize(200, 200);
  sidebar_layout->addSpacing(45);
  sidebar_layout->addWidget(close_btn, 0, Qt::AlignCenter);
  QObject::connect(close_btn, &QPushButton::clicked, this, &SettingsWindow::closeSettings);

  // setup panels
  DevicePanel *device = new DevicePanel(this);
  QObject::connect(device, &DevicePanel::reviewTrainingGuide, this, &SettingsWindow::reviewTrainingGuide);
  QObject::connect(device, &DevicePanel::showDriverView, this, &SettingsWindow::showDriverView);

  QList<QPair<QString, QWidget *>> panels = {
    {"Device", device},
    {"Network", network_panel(this)},
    {"Toggles", new TogglesPanel(this)},
    {"Software", new SoftwarePanel(this)},
  };

#ifdef ENABLE_MAPS
  auto map_panel = new MapPanel(this);
  panels.push_back({"Navigation", map_panel});
  QObject::connect(map_panel, &MapPanel::closeSettings, this, &SettingsWindow::closeSettings);
#endif

  const int padding = panels.size() > 3 ? 25 : 35;

  nav_btns = new QButtonGroup();
  for (auto &[name, panel] : panels) {
    QPushButton *btn = new QPushButton(name);
    btn->setCheckable(true);
    btn->setChecked(nav_btns->buttons().size() == 0);
    btn->setStyleSheet(QString(R"(
      QPushButton {
        color: grey;
        border: none;
        background: none;
        font-size: 65px;
        font-weight: 500;
        padding-top: %1px;
        padding-bottom: %1px;
      }
      QPushButton:checked {
        color: white;
      }
      QPushButton:pressed {
        color: #ADADAD;
      }
    )").arg(padding));

    nav_btns->addButton(btn);
    sidebar_layout->addWidget(btn, 0, Qt::AlignRight);

    const int lr_margin = name != "Network" ? 50 : 0;  // Network panel handles its own margins
    panel->setContentsMargins(lr_margin, 25, lr_margin, 25);

    ScrollView *panel_frame = new ScrollView(panel, this);
    panel_widget->addWidget(panel_frame);

    QObject::connect(btn, &QPushButton::clicked, [=, w = panel_frame]() {
      btn->setChecked(true);
      panel_widget->setCurrentWidget(w);
    });
  }
  sidebar_layout->setContentsMargins(50, 50, 100, 50);

  // main settings layout, sidebar + main panel
  QHBoxLayout *main_layout = new QHBoxLayout(this);

  sidebar_widget->setFixedWidth(500);
  main_layout->addWidget(sidebar_widget);
  main_layout->addWidget(panel_widget);

  setStyleSheet(R"(
    * {
      color: white;
      font-size: 50px;
    }
    SettingsWindow {
      background-color: black;
    }
  )");
}

void SettingsWindow::hideEvent(QHideEvent *event) {
#ifdef QCOM
  HardwareEon::close_activities();
#endif
}
