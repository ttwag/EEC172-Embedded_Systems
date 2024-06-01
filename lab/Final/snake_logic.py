import pygame

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

# Parameters of the snake
rect_x = 100
rect_y = 100
# Direction: 0, 1, 2, 3 - Right, Left, Up, Down
direction = 0 
snake_size = 25

snake = [[0, 0, 0], [snake_size, 0, 0], [2 * snake_size, 0, 0]]

# Game loop that keeps the window open
running = True
while running:
    
    # Check for events
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

    if (rect_x >= screen_width - wall_thickness - snake_size): 
        break
    if (rect_x <= wall_thickness):
        break
    if (rect_y >= screen_height - wall_thickness - snake_size):
        break
    if (rect_y <= wall_thickness):
        break

    # Delete the old rectangle by drawing over it with a black rectangle
    pygame.draw.rect(screen, BLACK, (rect_x, rect_y, snake_size, snake_size))

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
    elif direction == 0:
        rect_x += 5
    elif direction == 1:
        rect_x -= 5
    elif direction == 2:
        rect_y -= 5
    elif direction == 3:
        rect_y += 5
    else:
        rect_x += 5

   
    # Draw the new rectangle at its updated position
    pygame.draw.rect(screen, WHITE, (rect_x, rect_y, snake_size, snake_size))

    # Update the display
    pygame.display.flip()
    
    clock.tick(60)


# Quit Pygame
pygame.quit()


# Draw a snake

# Move the snake
