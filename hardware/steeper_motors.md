# Steeper motors

A stepper motor, also known as step motor or stepping motor, is a brushless DC electric motor that rotates in a series of small and discrete angular steps. Stepper motors can be set to any given step position without needing a position sensor for feedback. The step position can be rapidly increased or decreased to create continuous rotation, or the motor can be ordered to actively hold its position at one given step.

## The 28BYJ-48 Stepper Motor

From https://lastminuteengineers.com

The 28BYJ-48 is a small, 5-wire unipolar stepper motor that runs on 5 volts. It’s perfect for projects that need precise, controlled movement, like moving a robotic arm to exact positions.

This motor has a compact, round body that’s about 28 mm in diameter. It comes with two small flanges for attaching to other surfaces. The shaft has a “Double-D” shape, which means it’s partly flat on two sides.

Despite its small size, the motor delivers a solid amount of torque—around 34.3 mN·m, while rotating at approximately 15 RPM.

The red wire should always be connected to positive voltage (HIGH). When you connect any of the other four wires to negative voltage (LOW), that phase becomes energized and helps move the motor.

To make the motor spin, you energize these four phases in a specific pattern. This pattern is called a step sequence.

According to the datasheet, without any gears, each step would move the shaft by 11.25°. This means it would take 32 steps to complete one full rotation (since 360° ÷ 11.25° = 32 steps).

However, this motor includes a built-in gearbox with a 64:1 reduction ratio. This means the internal rotor must rotate 64 times before the output shaft completes just one full turn. With this gearbox, it now takes 32 × 64 = 2048 steps for the output shaft to make one complete rotation.

The 28BYJ-48 stepper motor runs on 5 volts but requires a fair amount of current—about 240 milliamps (mA) per phase.

It’s also important to note that stepper motors, including the 28BYJ-48, consume power even when not actively rotating. This is because they need to maintain their position and exert torque to resist external forces. To do this, the motor keeps a small amount of electric current flowing through its coils. This current creates a magnetic field that locks the motor’s shaft into place. The amount of current required to hold the position is known as holding current.