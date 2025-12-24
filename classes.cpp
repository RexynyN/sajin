#include <iostream>
#include <vector>
#include <string>
#include <memory> // Para smart pointers
#include <ctime>

// --- 1. INTERFACE (Classe Abstrata Pura) ---
// Define um "contrato". Qualquer coisa que implemente isso DEVE ter o método logar().
class ILogravel {
public:
    virtual void logar(const std::string& mensagem) const = 0; // Método puramente virtual
    virtual ~ILogravel() = default; // Destrutor virtual é crucial em interfaces
};

// --- 2. CLASSE BASE (Abstrata) ---
class Tarefa : public ILogravel {
protected: 
    // 'protected' permite que as classes filhas acessem, mas o mundo externo não.
    int id;
    std::string descricao;
    bool concluida;
    
    // Membro Estático: Compartilhado por TODAS as instâncias desta classe
    static int contadorTarefas; 

public:
    // Construtor
    explicit Tarefa(std::string desc) 
        : id(++contadorTarefas), descricao(std::move(desc)), concluida(false) {
    }

    // Destrutor Virtual: Garante que o destrutor da classe filha seja chamado
    virtual ~Tarefa() {
        std::cout << "[Memoria] Destruindo Tarefa ID " << id << std::endl;
    }

    // --- MÉTODOS VIRTUAIS (Polimorfismo) ---
    
    // Método puramente virtual: Torna esta classe ABSTRATA (não pode ser instanciada)
    virtual void executar() = 0; 

    // Método virtual comum: Pode ser sobrescrito, mas tem comportamento padrão
    virtual float calcularProgresso() const {
        return concluida ? 100.0f : 0.0f;
    }

    // --- MÉTODOS CONCRETOS ---
    int getId() const { return id; }
    
    // Implementação da Interface ILogravel
    void logar(const std::string& msg) const override {
        std::cout << "[LOG - Tarefa " << id << "]: " << msg << std::endl;
    }

    // Getter Estático
    static int getTotalTarefasCriadas() {
        return contadorTarefas;
    }

    // --- SOBRECARGA DE OPERADOR ---
    // Permite comparar duas tarefas usando '=='
    bool operator==(const Tarefa& outra) const {
        return this->id == outra.id;
    }
};

// Inicialização do membro estático
int Tarefa::contadorTarefas = 0;

// --- 3. CLASSE DERIVADA 1: Tarefa de Backup ---
class BackupBancoDados : public Tarefa {
private:
    std::string stringConexao;
    int tamanhoTotalMB;
    int processadoMB;

public:
    BackupBancoDados(std::string desc, std::string conexao, int tamanho)
        : Tarefa(std::move(desc)), stringConexao(std::move(conexao)), tamanhoTotalMB(tamanho), processadoMB(0) {}

    // Sobrescrita obrigatória (implementação da lógica específica)
    void executar() override {
        logar("Iniciando conexao com " + stringConexao);
        // Simulação de processo
        processadoMB = tamanhoTotalMB; 
        concluida = true;
        logar("Backup finalizado com sucesso.");
    }

    // Sobrescrita opcional (refinando o comportamento)
    float calcularProgresso() const override {
        if (tamanhoTotalMB == 0) return 0.0f;
        return (static_cast<float>(processadoMB) / tamanhoTotalMB) * 100.0f;
    }
};

// --- 4. CLASSE DERIVADA 2: Tarefa de Envio de Email ---
class EnvioEmail : public Tarefa {
private:
    std::string destinatario;

public:
    EnvioEmail(std::string desc, std::string email)
        : Tarefa(std::move(desc)), destinatario(std::move(email)) {}

    void executar() override {
        logar("Enviando email para " + destinatario);
        // Lógica de envio...
        concluida = true;
    }
    
    // Não sobrescreve calcularProgresso, usa o padrão da classe base (0 ou 100)
};

// --- 5. CLASSE GERENCIADORA (Composição) ---
class Agendador {
private:
    // Uso de unique_ptr para gerenciamento automático de memória (RAII)
    // Polimorfismo: O vetor guarda ponteiros para 'Tarefa', mas os objetos são 'Backup' ou 'Email'
    std::vector<std::unique_ptr<Tarefa>> filaDeTarefas;

public:
    void adicionarTarefa(std::unique_ptr<Tarefa> tarefa) {
        // move é necessário pois unique_ptr não pode ser copiado, apenas transferido
        filaDeTarefas.push_back(std::move(tarefa));
    }

    void processarTudo() {
        std::cout << "\n--- Iniciando Processamento do Agendador ---\n";
        
        // Loop range-based moderno
        for (const auto& tarefa : filaDeTarefas) {
            tarefa->executar(); // Chamada Polimórfica (o C++ decide qual 'executar' chamar em tempo de execução)
            std::cout << "Progresso: " << tarefa->calcularProgresso() << "%\n";
            std::cout << "--------------------------------\n";
        }
    }
};

// --- 6. FUNÇÃO PRINCIPAL ---
int main() {
    Agendador scheduler;

    std::cout << "Tarefas antes: " << Tarefa::getTotalTarefasCriadas() << std::endl;

    // Criando tarefas e passando a posse (ownership) para o agendador
    scheduler.adicionarTarefa(
        std::make_unique<BackupBancoDados>("Backup Diario", "DB_PROD_01", 5000)
    );

    scheduler.adicionarTarefa(
        std::make_unique<EnvioEmail>("Newsletter Semanal", "cliente@exemplo.com")
    );

    std::cout << "Tarefas depois: " << Tarefa::getTotalTarefasCriadas() << std::endl;

    scheduler.processarTudo();

    // Nota: Não precisamos chamar 'delete'. 
    // Quando 'scheduler' sair de escopo, o vector é destruído.
    // O vector destrói os unique_ptr, que destroem as Tarefas (chamando o destrutor virtual).
    
    return 0;
}