package com.example.demo;

import javafx.application.Application;
import javafx.geometry.Insets;
import javafx.scene.Scene;
import javafx.scene.control.TableView;
import javafx.scene.layout.VBox;
import javafx.scene.control.TableColumn;
import javafx.scene.control.cell.PropertyValueFactory;
import javafx.stage.Stage;
import javafx.geometry.Insets;

public class Main extends Application {
    @Override
    public void start(Stage stage) {
        TableView table = new TableView<ProcessInfo>();
        TableColumn<ProcessInfo,String> pid = new TableColumn<>("PID");
        pid.setCellValueFactory(new PropertyValueFactory<>("pid"));
        TableColumn<ProcessInfo,String> ppid = new TableColumn<>("PPID");
        ppid.setCellValueFactory(new  PropertyValueFactory<>("ppid"));
        TableColumn<ProcessInfo,String> name = new TableColumn<>("Name");
        name.setCellValueFactory(new PropertyValueFactory<>("name"));
        TableColumn<ProcessInfo,String> state = new TableColumn<>("State");
        state.setCellValueFactory(new PropertyValueFactory<>("state"));
        TableColumn<ProcessInfo,String> vmsize = new TableColumn<>("VMsize");
        vmsize.setCellValueFactory(new PropertyValueFactory<>("vmsize"));
        TableColumn<ProcessInfo,String> rss = new TableColumn<>("RSS");
        rss.setCellValueFactory(new PropertyValueFactory<>("rss"));
        TableColumn<ProcessInfo,String> cputime = new TableColumn<>("Cputime");
        cputime.setCellValueFactory(new PropertyValueFactory<>("cputime"));
        table.getColumns().addAll(pid,ppid,name,state,vmsize,rss,cputime);
        VBox root = new VBox();
        root.getChildren().addAll(table);
        root.setSpacing(10);
        root.setPadding(new Insets(10));
        Scene scene = new Scene(root, 800, 600);
        stage.setScene(scene);
        stage.setTitle("Prozessübersicht");
        stage.show();
    }

    public static void main(String[] args) {
        launch();
    }
}
