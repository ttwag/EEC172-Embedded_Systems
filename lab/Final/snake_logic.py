import pygame
import random

class ListNode:
    def __init__(self, coordinate):
        self.coordinate = coordinate
        self.prev = None
        self.next = None

class DoublyLinkedList:
    def __init__(self):
        self.head = None
        self.tail = None
        self.length = 0
    def insert_front(self, coordinate):
        new_Node = ListNode(coordinate)
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
            print(node.coordinate)
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
        head_x = self.snake_head.coordinate[0]
        head_y = self.snake_head.coordinate[1]
        if direction == 0:
            self.body.insert_front([head_x + self.snake_seg_size, head_y])
        elif direction == 1:
            self.body.insert_front([head_x - self.snake_seg_size, head_y])
        elif direction == 2:
            self.body.insert_front([head_x, head_y - self.snake_seg_size])
        else:
            self.body.insert_front([head_x, head_y + self.snake_seg_size])
        self.body.delete_tail()
        self.snake_head = self.body.head
        self.snake_tail = self.body.tail
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
PURPLE = (128, 0, 255)

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
last_direction = 3
snake_seg_size = 20
snake_length = 10

snake = Snake(snake_length, snake_seg_size, rect_x)
segment = snake.snake_head
snake.body.print_list()

while segment:
    pygame.draw.rect(screen, WHITE, (segment.coordinate[0], segment.coordinate[1], snake_seg_size, snake_seg_size))
    segment = segment.next

# Fruit
fruit = [-1, -1]

# Game loop that keeps the window open
running = True
while running:
    # Check for events
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

    
    # End the game if snake's head hits the wall
    if (snake.snake_head.coordinate[0] >= screen_width - wall_thickness - snake_seg_size - 1): 
        break
    if (snake.snake_head.coordinate[0] <= wall_thickness):
        break
    if (snake.snake_head.coordinate[1] >= screen_height - wall_thickness - snake_seg_size - 1):
        break
    if (snake.snake_head.coordinate[1] <= wall_thickness):
        break
    
    # Checks if the snake is on top of the fruit
    if abs(snake.snake_head.coordinate[0] - fruit[0]) <= 2 or abs(snake.snake_head.coordinate[1] - fruit[1]) <= 2:
        pygame.draw.rect(screen, BLACK, (fruit[0], fruit[1], snake_seg_size, snake_seg_size))    
        fruit = [-1, -1]

    # Delete the old rectangle by drawing over it with a black rectangle
    pygame.draw.rect(screen, BLACK, (snake.snake_tail.coordinate[0], snake.snake_tail.coordinate[1], snake_seg_size, snake_seg_size))

    # Randomly generate new fruit if not have one already
    if fruit == [-1, -1]:
        fruit = [random.randint(wall_thickness, screen_width - wall_thickness - snake_seg_size - 1), random.randint(wall_thickness, screen_width - wall_thickness - snake_seg_size - 1)]
        pygame.draw.rect(screen, PURPLE, (fruit[0], fruit[1], snake_seg_size, snake_seg_size))

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

    # Cannot move in the opposite direction
    if (last_direction == 0 and direction == 1) or (last_direction == 1 and direction == 0) or (last_direction == 2 and direction == 3) or (last_direction == 3 and direction == 2):
        direction = last_direction
    else: last_direction = direction

    # Update the snake's direction
    snake.move(direction)
    
    # Draw the new rectangle at its updated position
    pygame.draw.rect(screen, WHITE, (snake.snake_head.coordinate[0], snake.snake_head.coordinate[1], snake_seg_size, snake_seg_size))

    # Update the display
    pygame.display.flip()
    
    clock.tick(30)


# Quit Pygame
pygame.quit()