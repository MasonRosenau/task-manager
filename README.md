# Task Manager

This is a simple task manager designed for use on Linux systems. It allows users to import and export tasks, complete tasks, view tasks, and create new tasks.

## Importing Tasks:

If you decide to import tasks from a file, the file should be formatted with each task on a separate line in the following format:
```
x|Task Name|YYYY_MM_DD|Category
```
- `x` can take on a value of `0` or `1`.
  - `0` indicates that the task is incomplete.
  - `1` indicates that the task is complete.
- `Task Name` is the name of the task.
- `YYYY_MM_DD` is the due date of the task.
- `Category` is the category of the task.
  - If a task has no category, the category should be `None`.

### Example Import File

Below is an example of a file that can be imported into the task manager:
```
1|Submit assignment|2024_01_01|School
0|Go to the gym|1990_12_31|Personal
1|Read a book|2024_02_15|Personal
0|Complete coding project|2024_03_10|Work
1|Plan weekend trip|2024_04_20|Personal
[empty line]
```
Note that the file ends in a newline character. This is important when it comes to exporting tasks by appending to an existing file.

## Exporting Tasks

When it comes to exporting tasks from the Task Manager program to a file, there are two options as follows:

1. **Overwrite**: This option will overwrite all previous contents of the chosen file with the tasks in the Task Manager program.

2. **Append**: This option will append the tasks in the Task Manager program to the end of the chosen file.

In both cases, if the file does not exist, it will be created.