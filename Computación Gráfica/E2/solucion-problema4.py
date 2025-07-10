import numpy as np
import matplotlib.pyplot as plt
import random


def simulate_needle_drops(floor_size, strip_width, needle_length, num_simulations):
    intersections = 0
    separation_lines = np.arange(0, floor_size + strip_width, strip_width)

    for _ in range(num_simulations):
        margin = needle_length / 2
        center_x = random.uniform(margin, floor_size - margin)
        center_y = random.uniform(margin, floor_size - margin)
        angle = random.uniform(0, 2 * np.pi)

        half_length = needle_length / 2
        x1 = center_x - half_length * np.cos(angle)
        y1 = center_y - half_length * np.sin(angle)
        x2 = center_x + half_length * np.cos(angle)
        y2 = center_y + half_length * np.sin(angle)

        min_x = min(x1, x2)
        max_x = max(x1, x2)

        crossed = False
        for line_x in separation_lines:
            if min_x < line_x < max_x:
                crossed = True
                break

        if crossed:
            intersections += 1

    probability = intersections / num_simulations
    return probability, intersections


def theoretical_probability(strip_width, needle_length):
    return (2 * needle_length) / (np.pi * strip_width)


def run_simulation_study():
    floor_size = 1000
    strip_widths = [1, 5, 25]
    num_simulations = 100000

    results = []

    print("Simulación del Problema de la Aguja de Buffon")
    print("=" * 50)
    print(f"Piso: {floor_size}x{floor_size}")
    print(f"Número de simulaciones: {num_simulations}")
    print()

    for width in strip_widths:
        needle_length = width

        print(f"Ancho de listón: {width}")
        print(f"Largo de aguja: {needle_length}")

        prob_sim, intersections = simulate_needle_drops(
            floor_size, width, needle_length, num_simulations
        )

        prob_theoretical = theoretical_probability(width, needle_length)
        error = abs(prob_sim - prob_theoretical) / prob_theoretical * 100

        results.append({
            'width': width,
            'prob_sim': prob_sim,
            'prob_theoretical': prob_theoretical,
            'intersections': intersections,
            'error': error
        })

        print(f"  Intersecciones: {intersections}/{num_simulations}")
        print(f"  Probabilidad simulada: {prob_sim:.4f}")
        print(f"  Probabilidad teórica: {prob_theoretical:.4f}")
        print(f"  Error: {error:.2f}%")
        print()

    return results


def plot_results(results):
    widths = [r['width'] for r in results]
    prob_sim = [r['prob_sim'] for r in results]
    prob_theoretical = [r['prob_theoretical'] for r in results]
    errors = [r['error'] for r in results]

    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(15, 6))

    x_pos = np.arange(len(widths))
    width_bar = 0.35

    ax1.bar(x_pos - width_bar/2, prob_sim, width_bar,
            label='Simulación', alpha=0.8, color='skyblue')
    ax1.bar(x_pos + width_bar/2, prob_theoretical, width_bar,
            label='Teórica (Buffon)', alpha=0.8, color='lightcoral')

    ax1.set_xlabel('Ancho del Listón')
    ax1.set_ylabel('Probabilidad de Intersección')
    ax1.set_title('Comparación: Simulación vs Teoría')
    ax1.set_xticks(x_pos)
    ax1.set_xticklabels(widths)
    ax1.legend()
    ax1.grid(True, alpha=0.3)

    for i, (sim, theo) in enumerate(zip(prob_sim, prob_theoretical)):
        ax1.text(i - width_bar/2, sim + 0.01, f'{sim:.3f}',
                 ha='center', va='bottom', fontsize=9)
        ax1.text(i + width_bar/2, theo + 0.01, f'{theo:.3f}',
                 ha='center', va='bottom', fontsize=9)

    ax2.bar(x_pos, errors, color='orange', alpha=0.7)
    ax2.set_xlabel('Ancho del Listón')
    ax2.set_ylabel('Error Porcentual (%)')
    ax2.set_title('Error de la Simulación respecto a la Teoría')
    ax2.set_xticks(x_pos)
    ax2.set_xticklabels(widths)
    ax2.grid(True, alpha=0.3)

    for i, error in enumerate(errors):
        ax2.text(i, error + 0.1, f'{error:.2f}%',
                 ha='center', va='bottom', fontsize=10)

    plt.tight_layout()
    plt.savefig('buffon_needle_results.png', dpi=300, bbox_inches='tight')
    plt.close()


def visualize_needle_drops(strip_width, num_needles=50):
    fig, ax = plt.subplots(1, 1, figsize=(10, 10))

    area_size = 200
    needle_length = strip_width

    for x in range(0, area_size + strip_width, strip_width):
        ax.axvline(x, color='black', linewidth=2, alpha=0.7)

    intersecting_count = 0
    total_count = 0

    for _ in range(num_needles):
        margin = needle_length / 2
        center_x = random.uniform(margin, area_size - margin)
        center_y = random.uniform(margin, area_size - margin)
        angle = random.uniform(0, 2 * np.pi)

        half_length = needle_length / 2
        x1 = center_x - half_length * np.cos(angle)
        y1 = center_y - half_length * np.sin(angle)
        x2 = center_x + half_length * np.cos(angle)
        y2 = center_y + half_length * np.sin(angle)

        min_x = min(x1, x2)
        max_x = max(x1, x2)

        intersects = False
        for line_x in range(0, area_size + strip_width, strip_width):
            if min_x < line_x < max_x:
                intersects = True
                break

        if intersects:
            intersecting_count += 1
        total_count += 1

        color = 'red' if intersects else 'blue'
        ax.plot([x1, x2], [y1, y2], color=color, linewidth=2, alpha=0.7)

    ax.set_xlim(0, area_size)
    ax.set_ylim(0, area_size)
    ax.set_aspect('equal')
    ax.set_title(f'Agujas sobre Listones (Ancho: {strip_width})\n' +
                 f'Rojas: Intersectan ({intersecting_count}/{total_count}), Azules: No intersectan')
    ax.grid(True, alpha=0.3)

    plt.savefig(
        f'buffon_visualization_width_{strip_width}.png', dpi=300, bbox_inches='tight')
    plt.close()


if __name__ == "__main__":
    results = run_simulation_study()
    plot_results(results)

    for width in [1, 5, 25]:
        visualize_needle_drops(width, num_needles=30)

    print("Gráficos guardados:")
    print("- buffon_needle_results.png (comparación principal)")
    print("- buffon_visualization_width_1.png")
    print("- buffon_visualization_width_5.png")
    print("- buffon_visualization_width_25.png")
