import pygame

from agents import HumanStatus

FRAMES_PER_SECOND = 10


class Display:
    def __init__(self, simulation, width, height):
        pygame.init()
        self.simulation = simulation
        self.width = width
        self.height = height

        self.screen = pygame.display.set_mode((width, height))
        pygame.display.set_caption("Agent Movement Simulation")
        self.clock = pygame.time.Clock()

        self.bg_color = (255, 255, 255)

    def draw_text(self, text, pos, color=(0, 0, 0)):
        font = pygame.font.SysFont(None, 15)
        text_surf = font.render(text, True, color)
        self.screen.blit(text_surf, pos)

    # Returns whether the simulation should continue running
    def render(self) -> bool:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                return False

        self.screen.fill(self.bg_color)

        for agent in self.simulation.animal_agents:
            screen_x = int(agent.location.x)
            screen_y = int(agent.location.y)

            color = (0, 200, 0)  # green for animals
            radius = agent.radius

            pygame.draw.circle(self.screen, color, (screen_x, screen_y), radius)
            self.draw_text(
                f"A{agent.id}", (screen_x, screen_y - (radius + 10)), color=color
            )

        for agent in self.simulation.human_agents.values():
            screen_x = int(agent.location.x)
            screen_y = int(agent.location.y)

            if agent.status == HumanStatus.SICK:
                color = (255, 0, 0)  # sick human = red
            else:
                color = (0, 0, 255)  # healthy human = blue

            radius = 5

            pygame.draw.circle(self.screen, color, (screen_x, screen_y), radius)
            self.draw_text(
                f"H{agent.id}", (screen_x, screen_y - (radius + 10)), color=color
            )

        self.draw_text(f"Sim time: t={self.simulation.time_step}", (10, 10))
        self.draw_text(
            f"Real time: t={self.simulation.get_current_real_time()}s", (10, 20)
        )

        pygame.display.flip()
        self.clock.tick(FRAMES_PER_SECOND)

        return True

    def cleanup(self):
        pygame.quit()
