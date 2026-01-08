#include "factory.hpp"
#include "reports.hpp"
#include "simulation.hpp"
#include <iostream>
#include <fstream>

int main() {
    std::cout << "NetSim Simulation starting..." << std::endl;
    
    // 1. WCZYTYWANIE STRUKTURY Z PLIKU
    std::ifstream input_file("factory_structure.txt");
    if (!input_file.is_open()) {
        std::cerr << "Error: Could not open factory_structure.txt" << std::endl;
        return 1;
    }

    try {
        NetSim::Factory factory = NetSim::load_factory_structure(input_file);
        std::cout << "Factory structure loaded successfully." << std::endl;

        // 2. MODYFIKACJA STRUKTURY Z POZIOMU KODU (PRZYKŁAD)
        // Możemy dodawać nowe węzły lub usuwać istniejące.
        // factory.add_worker(NetSim::Worker(3, 1, std::make_unique<NetSim::PackageQueue>(NetSim::PackageQueueType::FIFO)));
        
        // Możemy również ręcznie dodawać połączenia (LINK):
        // auto worker3 = factory.find_worker_by_id(3);
        // auto storehouse1 = factory.find_storehouse_by_id(1);
        // if (worker3 != factory.worker_cend() && storehouse1 != factory.storehouse_cend()) {
        //     worker3->get_receiver_preferences().add_receiver(&*storehouse1);
        // }

        // 3. WERYFIKACJA SPÓJNOŚCI i RAPORT STRUKTURY
        if (factory.is_consistent()) {
            std::cout << "Network is consistent." << std::endl;
        } else {
            std::cerr << "Warning: Network is inconsistent!" << std::endl;
        }
        
        std::cout << "\n--- Initial Factory Structure ---\n" << std::endl;
        NetSim::generate_structure_report(factory, std::cout);

        // 4. ZAPIS STRUKTURY DO PLIKU
        std::ofstream output_file("factory_structure_saved.txt");
        if (output_file.is_open()) {
            NetSim::save_factory_structure(factory, output_file);
            std::cout << "Structure saved to factory_structure_saved.txt" << std::endl;
        }
        
        // 5. KONFIGURACJA RAPORTOWANIA
        // Opcja A: Raport co N-tą turę (np. co 1)
        NetSim::IntervalReportNotifier notifier(1); 
        
        // Opcja B: Raport w konkretnych turach (np. 1 i 5)
        // NetSim::SpecificTurnsReportNotifier notifier({1, 5});

        // 6. URUCHOMIENIE SYMULACJI
        std::cout << "\n--- Running Simulation ---\n" << std::endl;
        NetSim::simulate(factory, 5, [&notifier](NetSim::Factory& f, NetSim::Time t) {
            if (notifier.should_generate_report(t)) {
                NetSim::generate_simulation_turn_report(f, std::cout, t);
            }
        });

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "NetSim Simulation finished." << std::endl;
    return 0;
}
