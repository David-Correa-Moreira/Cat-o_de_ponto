{{-- resources/views/admin/dashboard.blade.php --}}

@extends('adminlte::page')

@section('title', 'Novo Usuário')

{{--@section('content_header')
    <h1>Inserir Usuário</h1>
@stop--}}

@section('content')
    <div class="card">
        <h4 class="card-header">Insira Novo Usuário</h4>
        <div class="card-body">
            <form action="{{ route('registers.store') }}" method="POST">
                @csrf
                @include('control.projects.registers._partials.form')
            </form>
        </div>
    </div>
@stop