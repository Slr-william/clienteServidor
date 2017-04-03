
import zmq
import random
import sys
import hashlib
import json
import string


def get_id(simple=True, length=6):
    if (simple):
        possible = list(string.ascii_lowercase)
        rnd = random.randint(0, len(possible))
        return str(possible[rnd])
    else:
        rnd = random.randint(0, 100)
        id = hashlib.sha1()
        id.update(str(rnd))
        return str(id.hexdigest())[:length]


def get_hash(val):
    id = hashlib.sha1()
    id.update(val)
    return str(id.hexdigest())[:6]


def main():
    if len(sys.argv) != 2:
        print('Enter configuration file')

    configFile = open(sys.argv[1], 'r')
    config = json.load(configFile)

    client_name = get_id()
    print("Starting node with name {}".format(client_name))

    context = zmq.Context()
    request = context.socket(zmq.PUSH)
    requestAddress = 'tcp://localhost:{}'.format(config['server'])
    request.connect(requestAddress)
    print('Connect to server on {} and listen for answers on {}'.format(config['server'], config['client']))

    answers = context.socket(zmq.PULL)
    answersAddress = 'tcp://*:{}'.format(config['client'])
    answers.bind(answersAddress)

    poller = zmq.Poller()
    stdin = sys.stdin.fileno()
    poller.register(stdin, zmq.POLLIN)
    poller.register(answers, zmq.POLLIN)

    print('Send something!')
    should_continue = True
    while should_continue:
        print('Enter your option (insert, search or delete):')
        socks = dict(poller.poll())
        if stdin in socks and socks[stdin] == zmq.POLLIN:
            option = raw_input()
            if option == "insert":
                val = raw_input("Enter value to insert")
                print("Input is: " + val)
                message = {
                    'id': client_name,
                    'answer_to': 'tcp://localhost:{}'.format(config['client']),
                    'type': 'insert',
                    'key': val,  # get_hash(val),
                    'value': val
                    }
                request.send_json(message)
                print(answers.recv())
            elif option == "search":
                val = raw_input("Enter value to search")
                print("Input is: " + val)
                message = {
                    'id': client_name,
                    'answer_to': 'tcp://localhost:{}'.format(config['client']),
                    'type': 'search',
                    'key': val,  # get_hash(val),
                    }
                request.send_json(message)
                print(answers.recv_json())
            elif option == "delete":
                val = raw_input("Enter value to delete")
                print("Input is: " + val)
                message = {
                    'id': client_name,
                    'answer_to': 'tcp://localhost:{}'.format(config['client']),
                    'type': 'delete',
                    'key': val,  # get_hash(val),
                    }
                request.send_json(message)
                print(answers.recv())
            else:
                print("Please, enter a correct option")
        if answers in socks and socks[answers] == zmq.POLLIN:
            print("Data on answers!!!")
            req = answers.recv_json()
            if req['status']:
                print('Server:', req['hash'])
            else:
                print('Error:', req['message'])
        should_continue = True


if __name__ == '__main__':
    main()
