idf_component_register(
    SRCS
        "../../ace/axis/axis_manager.cpp"
        "../../ace/communication/alp.cpp"
        "../../ace/diagnostics/diagnostics.cpp"
        "../../ace/control/controller.cpp"
        "../../ace/drivers/encoder.cpp"
        "../../ace/hal/hal.cpp"
        "../../ace/config/persistent_storage.cpp"
        "../../ace/motion/motion_planner.cpp"
        "../../ace/scheduler/scheduler.cpp"
        "../../ace/safety/safety_manager.cpp"
        "../../ace/services/logger.cpp"
        "../../ace/telemetry/telemetry_publisher.cpp"
    INCLUDE_DIRS "../../"
)
