# simple-logger
A script that logs the output of two programs and can send the output via Email.(On UNIX-based Systems)
## Details
The Script automatically creates 2 files (.txt) that are stored locally and executes two other (user defined) programs. These programs are being executed independently and upon finishing their outputs are being written in their seperate files. The script contains an infinite loop that executes the two programms after a (user defined) delay. Also,the two Emails that are being sent are the text files that contain the program outputs. The Email send is being triggered by a counter that counts how many times the scripts have executed and once this counter exceeds the set number of executions.There is also an time-triggered end of execution of the script.For the email to work,the MailX service must be installed and configured. 
### How to use
- Edit and place the parameters required to run your scripts(like some of the definitions that already exist,Email of the receipient etc).
- Compile and run!
