import pygame

class ListNode:
    def __init__(self, data):
        self.data = data
        self.prev = None
        self.next = None

class DoublyLinkedList:
    def __init__(self):
        self.head = None
        self.tail = None
        self.length = 0
    def insert_front(self, data):
        new_Node = ListNode(data)
        new_Node.prev = None
        if self.length:
            new_Node.next = self.head
            self.head.prev = new_Node
            self.head = new_Node
        else:
            new_Node.next = None
            self.head = new_Node
            self.tail = new_Node
        self.length+=1
    def delete_tail(self):
        if self.length > 1:
            self.length-=1
            self.tail = self.tail.prev
            self.tail.next = None
        else:
            self.length = 0
            self.head = None
            self.tail = None
    def print_list(self):
        node = self.head
        while node:
            print(node.data)
            node = node.next
class Snake:
    def __init__(self, length, seg_size, init_pos):
        self.snake_seg_size = seg_size
        self.body = DoublyLinkedList()
        for i in range(length-1, -1, -1):
            self.body.insert_front([init_pos + i * seg_size, init_pos])
        self.snake_head = self.body.head
        self.snake_tail = self.body.tail
    def move(self, direction):
        head_x = self.head.data[0]
        head_y = self.head.data[1]
        if direction == 0:
            self.body.insert_front([head_x + self.snake_seg_size, head_y])
        elif direction == 1:
            self.body.insert_front([head_x - self.snake_seg_size, head_y])
        elif direction == 2:
            self.body.insert_front([head_x, head_y - self.snake_seg_size])
        else:
            self.body.insert_front([head_x, head_y + self.snake_seg_size])
        self.body.delete_tail()
    def self_collision(self):
        pass


# Open a window
pygame.init()
screen_width = 800
screen_height = 800
screen = pygame.display.set_mode((screen_width, screen_height))
pygame.display.set_caption("Snake Game")
clock = pygame.time.Clock()

# Set color
Color = (255, 0, 0)
BLACK = (0, 0, 0)
WHITE = (255, 255, 255)

# Define wall dimensions
wall_thickness = 20

# Draw top wall
pygame.draw.rect(screen, Color, (0, 0, screen_width, wall_thickness))

# Draw bottom wall
pygame.draw.rect(screen, Color, (0, screen_height - wall_thickness, screen_width, wall_thickness))

# Draw left wall
pygame.draw.rect(screen, Color, (0, 0, wall_thickness, screen_height))

# Draw right wall
pygame.draw.rect(screen, Color, (screen_width - wall_thickness, 0, wall_thickness, screen_height))

# Build a snake
# Parameters of the snake
rect_x = 100
rect_y = 100

# Direction: 0, 1, 2, 3 - Right, Left, Up, Down
direction = 3
snake_seg_size = 20
snake_length = 15

snake = []
for i in range(0, snake_length):
    snake.append([rect_x + snake_seg_size * i, rect_y, 1])

# Initialize a snake object
# Delete the snake tail
# Move the snake
# Draw the new snake head


# Game loop that keeps the window open
running = True
while running:
    
    # Check for events
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

    # End the game if snake's head hits the wall
    if (snake[0][0] >= screen_width - wall_thickness - snake_seg_size): 
        break
    if (snake[0][0] <= wall_thickness):
        break
    if (snake[0][1] >= screen_height - wall_thickness - snake_seg_size):
        break
    if (snake[0][1] <= wall_thickness):
        break
    
    
    # Delete the old rectangle by drawing over it with a black rectangle
    for segment in snake:
        pygame.draw.rect(screen, BLACK, (segment[0], segment[1], snake_seg_size, snake_seg_size))
    
    # Randomly generate new food if not have one already

    # Move the rectangle depending on the pressed key and previous direction
    keys = pygame.key.get_pressed()
    if keys[pygame.K_RIGHT]:
        direction = 0
    elif keys[pygame.K_LEFT]:
        direction = 1
    elif keys[pygame.K_UP]:
        direction = 2
    elif keys[pygame.K_DOWN]:
        direction = 3

    last_direction = snake[0][2]

    # If currently moving direction is parallel to chosen direction, then direction doesn't change
    if not ((snake[0][2] == 0 and direction == 1) or (snake[0][2] == 1 and direction == 0) or (snake[0][2] == 2 and direction == 3) or (snake[0][2] == 3 and direction == 2)):        
        snake[0][2] = direction
        last_direction = direction
    
    # Record the new position of the head and checks if there's collision with other segments
    # If the new position 
    for segment in snake:
        curr_direction = segment[2]
        if curr_direction == 0:
            segment[0] += snake_seg_size
        elif curr_direction == 1:
            segment[0] -= snake_seg_size
        elif curr_direction == 2:
            segment[1] -= snake_seg_size
        elif curr_direction == 3:
            segment[1] += snake_seg_size
        else:
            segment[0] += snake_seg_size

        temp = segment[2]
        segment[2] = last_direction
        last_direction = temp
    
        # Draw the new rectangle at its updated position
        pygame.draw.rect(screen, WHITE, (segment[0], segment[1], snake_seg_size, snake_seg_size))
    


    # Update the display
    pygame.display.flip()
    
    clock.tick(30)


# Quit Pygame
pygame.quit()