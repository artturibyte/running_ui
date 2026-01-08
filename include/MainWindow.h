#pragma once

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QMessageBox>
#include <vector>
#include <string>

struct RunningEntry {
    std::string date;
    double kilometers;
    
    RunningEntry(std::string d, double km) : date(std::move(d)), kilometers(km) {}
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void on_add_button_clicked();

private:
    // Helper methods
    std::string get_current_date() const;
    void update_list_view();
    void update_statistics();
    void setup_ui();
    void save_to_file();
    void load_from_file();

    // Widgets
    QLineEdit *m_kilometers_entry;
    QPushButton *m_add_button;
    QLabel *m_total_label;
    QLabel *m_average_label;
    QLabel *m_count_label;
    QTextEdit *m_list_view;
    
    // Data storage
    std::vector<RunningEntry> m_entries;
    QString m_data_file;
};
