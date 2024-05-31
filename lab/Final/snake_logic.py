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

# x = 30
# old_x = 30

rect_x = 100
rect_y = 100

# Game loop that keeps the window open
running = True
while running:
    # Check for events
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
    
    # Clear the screen
    # screen.fill(BLACK)

    # Delete the old rectangle by drawing over it with a black rectangle
    pygame.draw.rect(screen, BLACK, (rect_x, rect_y, 50, 50))

    # Move the rectangle horizontally
    rect_x += 5  # Adjust the movement speed as needed

    # Draw the new rectangle at its updated position
    pygame.draw.rect(screen, WHITE, (rect_x, rect_y, 50, 50))

    # Update the display
    pygame.display.flip()
    
    clock.tick(60)

# Quit Pygame
pygame.quit()


# Draw a snake

# Move the snake
