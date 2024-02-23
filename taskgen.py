import time
import random
from datetime import date

verbs = ["Walk", "Write", "Eat", "Cook", "Go to", "Read", "Plan", "Draw", "Code", "Complete"]
nouns = ["the dog", "a paper", "dinner", "the store", "a book", "a vacation", "a picture", "an assignment", "a task"]
categories = ["Work", "School", "Personal"]

def generate_tasks(num_tasks):
    ret_str = ""
    for x in range(num_tasks):
        done = random.randint(0, 1) # generate random boolean of whether task is done
        verb = verbs[random.randint(0, len(verbs) - 1)]
        noun = nouns[random.randint(0, len(nouns) - 1)]
        day = date.fromordinal(random.randint(600000, 800000)) # randint day after 1.1.0001
        category = categories[random.randint(0, len(categories) - 1)]
        ret_str += str(done) + "|" + verb + " " + noun + "|" + day.strftime("%Y_%m_%d") + "|" + category + "\n"
    return ret_str

while 1:
    file = open("task-pipe.txt", "r+") # read and write
    if (file.read() == "gen"):
        # print("Generating 5 example tasks...")
        time.sleep(2) # wait 2 seconds so we can see the change 
        tasks = generate_tasks(5)
        # overwrite existing content of the file with rand
        file.truncate(0) # clear contents
        file.seek(0) # move back to beginning
        file.write(tasks) # write tasks
    file.close()
    time.sleep(2) # check every 2 seconds for changes to the file